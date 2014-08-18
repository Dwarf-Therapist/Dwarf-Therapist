/*
Dwarf Therapist
Copyright (c) 2009 Trey Stout (chmod)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
#include <QtGui>
#include <QtDebug>

#include "Foundation/NSAutoreleasePool.h"
#include "Foundation/NSFileManager.h"
#include "Foundation/NSBundle.h"
#include "AppKit/NSWorkspace.h"

#include "cp437codec.h"
#include "dfinstance.h"
#include "dfinstanceosx.h"
#include "defines.h"
#include "dwarf.h"
#include "utils.h"
#include "gamedatareader.h"
#include "memorylayout.h"
#include "memorysegment.h"
#include "truncatingfilelogger.h"
#include <stdio.h>
#include <mach/vm_map.h>
#include <mach/mach_traps.h>
#include <mach-o/dyld.h>
#include <mach/mach.h>
#include <mach/vm_region.h>
#include <mach/vm_statistics.h>
#include <mach/task.h>

#define MACH64 (MAC_OS_X_VERSION_MAX_ALLOWED >= 1040)

#if MACH64
#include <mach/mach_vm.h>
#else /* ! MACH64 */
#define mach_vm_size_t vm_size_t
#define mach_vm_address_t vm_address_t
#define mach_vm_read vm_read
#define mach_vm_write vm_write
#define mach_vm_region vm_region
#define VM_REGION_BASIC_INFO_COUNT_64 VM_REGION_BASIC_INFO_COUNT
#define VM_REGION_BASIC_INFO_64 VM_REGION_BASIC_INFO
#endif /* MACH64 */

struct STLStringHeader {
    quint32 length;
    quint32 capacity;
    qint32 refcnt;
};

DFInstanceOSX::DFInstanceOSX(QObject* parent)
    : DFInstance(parent)
{
}

DFInstanceOSX::~DFInstanceOSX() {
    if(m_attach_count > 0) {
        detach();
    }
}

QVector<uint> DFInstanceOSX::enumerate_vector(const uint &addr) {
    QVector<uint> addrs;
    if(!addr)
        return addrs;
    if(!attach())
        return addrs;

    VIRTADDR tmp_addr = 0;
    VIRTADDR start = read_addr(addr);
    VIRTADDR end = read_addr(addr + 4);
    int bytes = end - start;
    if(check_vector(start,end,addr)){
        QByteArray data(bytes, 0);
        int bytes_read = read_raw(start, bytes, data);
        if (bytes_read != bytes && m_layout->is_complete()) {
            LOGW << "Tried to read" << bytes << "bytes but only got" << bytes_read;
            detach();
            return addrs;
        }
        else if (bytes_read == -1) {
            LOGW << "Failed to read" << hexify(start);
            detach();
            return addrs;
        }
        for(int i = 0; i < bytes; i += 4) {
            tmp_addr = decode_dword(data.mid(i, 4));
            addrs << tmp_addr;
        }
    }
    detach();
    return addrs;
}

QString DFInstanceOSX::read_string(const VIRTADDR &addr) {
    VIRTADDR buffer_addr = read_addr(addr);
    int upper_size = 256;
    QByteArray buf(upper_size, 0);
    read_raw(buffer_addr, upper_size, buf);
    //int bytes_read = read_raw(buffer_addr, upper_size, buf);
    //if (bytes_read == -1) {
        //LOGW << "Failed to read from" << hexify(addr);
        //throw -1;
    //}

    buf.truncate(buf.indexOf(QChar('\0')));
    CP437Codec *c = new CP437Codec();
    return c->toUnicode(buf);
}

int DFInstanceOSX::write_string(const VIRTADDR &addr, const QString &str) {
    return 0; //the code below can potentially crash DF, which is unacceptable

    // Ensure this operation is done as one transaction
    attach();
    uintptr_t buffer_addr = get_string(str);
    if (buffer_addr) {
        // 1: This unavoidably leaks the old buffer; our own
        //    cannot be deallocated anyway.
        // 2: The string is truncated to sizeof(VIRTADDR),
        //    but come on, no-one will use strings that long :)
        write_raw(addr, sizeof(VIRTADDR), &buffer_addr);
    }
    detach();
    return buffer_addr ? str.length() : 0;
}

int DFInstanceOSX::write_int(const VIRTADDR &addr, const int &val) {
    return write_raw(addr, sizeof(int), (void*)&val);
}

QString DFInstanceOSX::calculate_checksum() {
    // ELF binaries don't seem to store a linker timestamp, so just MD5 the file.
    QFile proc(QString("/proc/%1/exe").arg(m_pid));
    QCryptographicHash hash(QCryptographicHash::Md5);
    if (!proc.open(QIODevice::ReadOnly) || !hash.addData(&proc)) {
        LOGE << "FAILED TO READ DF EXECUTABLE";
        return QString("UNKNOWN");
    }
    QString md5 = hexify(hash.result().mid(0, 4).toLower());
    TRACE << "GOT MD5:" << md5;
    return md5;
}

bool DFInstanceOSX::attach() {
    kern_return_t result;
    if(m_attach_count > 0) {
        m_attach_count++;
        return true;
    }

    result = task_suspend(m_task);
    if ( result != KERN_SUCCESS ) {
        return false;
    }
    m_attach_count++;
    return true;
}

bool DFInstanceOSX::detach() {
    kern_return_t result;

    if( m_attach_count == 0 ) {
        return true;
    }

    if( m_attach_count > 1 ) {
        m_attach_count--;
        return true;
    }

    result = task_resume(m_task);
    if ( result != KERN_SUCCESS ) {
        return false;
    }
    m_attach_count--;
    return true;
}

int DFInstanceOSX::read_raw(const VIRTADDR &addr, size_t bytes, QByteArray &buffer) {
    kern_return_t result;
    vm_size_t readsize = 0;
    int bytes_read = 0; // tracks how much we've read of what was asked for
    int step_size = 0x1000; // how many bytes to read each step
    QByteArray chunk(step_size, 0); // our temporary memory container
    buffer.fill(0, bytes); // zero our buffer

    // try to attach, will be ignored if we're already attached
    attach();
    for(VIRTADDR ptr = addr; ptr < addr + bytes; ptr += step_size) {
        if (ptr + step_size > addr + bytes) {
            step_size = addr + bytes - ptr;
        }
        result = vm_read_overwrite(m_task, ptr, step_size,
                          (vm_address_t) chunk.data(),
                          &readsize);
        //if ( result != KERN_SUCCESS ) {
            //LOGW << "Unable to read " << bytes << " byte(s) from " <<
            //hexify(addr) << "into buffer" << &buffer << endl;
            //detach();
            //return -1;
        //}
        buffer.replace(bytes_read, chunk.size(), chunk);
        bytes_read += readsize;
    }
    detach();
    return bytes_read;
}

int DFInstanceOSX::write_raw(const VIRTADDR &addr, const size_t &bytes, void *buffer) {
    kern_return_t result;

    attach();
    result = vm_write( m_task,  addr,  (vm_offset_t)buffer,  bytes );
    //if ( result != KERN_SUCCESS ) {
        //LOGW << "Unable to write " << bytes << " byte(s) from " <<
        //hexify(addr) << "into buffer" << &buffer << endl;
        //detach();
        //return -1;
    //}
    detach();
    return bytes;
}

bool DFInstanceOSX::find_running_copy(bool connect_anyway) {
    NSAutoreleasePool *authPool = [[NSAutoreleasePool alloc] init];

    NSWorkspace *workspace = [NSWorkspace sharedWorkspace];
    NSArray *launchedApps = [workspace launchedApplications];

    unsigned i, len = [launchedApps count];

    // compile process array
    for ( i = 0; i < len; i++ ) {
        NSDictionary *application = [launchedApps objectAtIndex:i];
        if ( [[application objectForKey:@"NSApplicationName"]
                                            isEqualToString:@"dwarfort.exe" ]) {

            m_loc_of_dfexe = QString( [[application objectForKey:
                                     @"NSApplicationPath"] UTF8String]);
            m_pid = [[application objectForKey:
                                            @"NSApplicationProcessIdentifier"]
                                            intValue];
            LOGI << "Found running copy, pid:" << m_pid << "path:" << m_loc_of_dfexe;
        }
    }

    kern_return_t kret = task_for_pid( current_task(), m_pid, &m_task );
    if (m_pid == 0 || ! (kret == KERN_SUCCESS)) {
        QMessageBox::warning(0, tr("Warning"),
                             tr("Unable to locate a running copy of Dwarf "
                                "Fortress, are you sure it's running?"));
        LOGW << "can't find running copy";
        m_is_ok = false;
        return m_is_ok;

    }

    m_is_ok = true;
    map_virtual_memory();
    m_layout = get_memory_layout(calculate_checksum(), !connect_anyway);

    [authPool release];

    return true;
}

void DFInstanceOSX::map_virtual_memory() {
    foreach(MemorySegment *seg, m_regions) {
        delete(seg);
    }
    m_regions.clear();
    if (!m_is_ok)
        return;

    kern_return_t result;

    mach_vm_address_t address = 0x0;
    mach_vm_size_t size = 0;
    vm_region_basic_info_data_64_t info;
    mach_msg_type_number_t infoCnt = VM_REGION_BASIC_INFO_COUNT_64;
    mach_port_t object_name = 0;

    m_lowest_address = 0;
    do
    {
        // get the next region
        result = mach_vm_region( m_task, &address, &size, VM_REGION_BASIC_INFO_64, (vm_region_info_t)(&info), &infoCnt, &object_name );

        if ( result == KERN_SUCCESS ) {
            if ((info.protection & VM_PROT_READ) == VM_PROT_READ  && (info.protection & VM_PROT_WRITE) == VM_PROT_WRITE) {
                MemorySegment *segment = new MemorySegment("", address, address+size);
                TRACE << "Adding segment: " << address << ":" << address+size << " prot: " << info.protection;
                m_regions << segment;

                if(m_lowest_address == 0)
                    m_lowest_address = address;
                if(address < m_lowest_address)
                    m_lowest_address = address;
                if((address + size) > m_highest_address)
                    m_highest_address = (address + size);
            }
        }

        address = address + size;
    } while (result != KERN_INVALID_ADDRESS);
    LOGD << "Mapped " << m_regions.size() << " memory regions.";
}

uintptr_t DFInstanceOSX::get_string(const QString &str) {
    if (m_string_cache.contains(str))
        return m_string_cache[str];

    CP437Codec *c = new CP437Codec();
    QByteArray data = c->fromUnicode(str);

    STLStringHeader header;
    header.capacity = header.length = data.length();
    header.refcnt = 1000000; // huge refcnt to avoid dealloc

    QByteArray buf((char*)&header, sizeof(header));
    buf.append(data);
    buf.append(char(0));

    uintptr_t addr = (uintptr_t) malloc(buf.length());
    if (addr) {
        write_raw(addr, buf.length(), buf.data());
        addr += sizeof(header);
    }

    return m_string_cache[str] = addr;
}

bool DFInstance::authorize() {
    // Create authorization reference
    OSStatus status;
    AuthorizationRef authorizationRef;

    char therapistExe[1024];
    uint32_t size = sizeof(therapistExe);
    _NSGetExecutablePath(therapistExe, &size);
    //NSLog(@"Therapist path: %s\n", therapistExe);

    if( DFInstanceOSX::isAuthorized() ) {
        // Ensure we're in the correct path
        QDir dir(therapistExe);
        dir.makeAbsolute();
        dir.cdUp();
        chdir(dir.absolutePath().toLocal8Bit());
        fflush(stdout);

        // Authorization for remote memory access on OS X trips Qt's setuid detection
        QCoreApplication::setSetuidAllowed(true);

        return true;
    }

    // AuthorizationCreate and pass NULL as the initial
    // AuthorizationRights set so that the AuthorizationRef gets created
    // successfully, and then later call AuthorizationCopyRights to
    // determine or extend the allowable rights.
    // http://developer.apple.com/qa/qa2001/qa1172.html
    status = AuthorizationCreate(NULL, kAuthorizationEmptyEnvironment,
                                 kAuthorizationFlagDefaults, &authorizationRef);
    if (status != errAuthorizationSuccess) {
        NSLog(@"Error: %d", status);
        return false;
    }

    // kAuthorizationRightExecute == "system.privilege.admin"
    AuthorizationItem right = {kAuthorizationRightExecute, 0, NULL, 0};
    AuthorizationRights rights = {1, &right};
    AuthorizationFlags flags = kAuthorizationFlagDefaults |
                               kAuthorizationFlagInteractionAllowed |
                               kAuthorizationFlagPreAuthorize |
                               kAuthorizationFlagExtendRights;

    // Call AuthorizationCopyRights to determine or extend the allowable rights.
    status = AuthorizationCopyRights(authorizationRef, &rights, NULL, flags, NULL);
    if (status != errAuthorizationSuccess) {
        NSLog(@"Error: %d", status);
        return false;
    }

    FILE *pipe = NULL;
    char readBuffer[32];

    status = AuthorizationExecuteWithPrivileges(authorizationRef, therapistExe,
                                                kAuthorizationFlagDefaults, nil, &pipe);
    if (status != errAuthorizationSuccess) {
        NSLog(@"Error: %d", status);
        return false;
    }

    // external app is running asynchronously
    // - it will send to stdout when loaded*/
    if (status == errAuthorizationSuccess)
    {
        read (fileno (pipe), readBuffer, sizeof (readBuffer));
        fclose(pipe);
    }

    // The only way to guarantee that a credential acquired when you
    // request a right is not shared with other authorization instances is
    // to destroy the credential.  To do so, call the AuthorizationFree
    // function with the flag kAuthorizationFlagDestroyRights.
    // http://developer.apple.com/documentation/Security/Conceptual/authorization_concepts/02authconcepts/chapter_2_section_7.html
    status = AuthorizationFree(authorizationRef, kAuthorizationFlagDestroyRights);
    return false;
}

bool DFInstanceOSX::isAuthorized() {
    // already authorized?
    AuthorizationRef myAuthRef;
    OSStatus stat = AuthorizationCopyPrivilegedReference(&myAuthRef,kAuthorizationFlagDefaults);

    bool ret = (stat == errAuthorizationSuccess || checkPermissions());
    if(!ret)
        NSLog(@"Not authorized");
    return ret;
}

bool DFInstanceOSX::checkPermissions() {
    NSAutoreleasePool *authPool = [[NSAutoreleasePool alloc] init];
    NSDictionary *applicationAttributes = [[NSFileManager defaultManager] fileAttributesAtPath:[[NSBundle mainBundle] executablePath] traverseLink: YES];
    return ([applicationAttributes filePosixPermissions] == 1517 && [[applicationAttributes fileGroupOwnerAccountName] isEqualToString: @"procmod"]);
    [authPool release];
}
