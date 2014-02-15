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
    if (!addr)
        return addrs;

    if( !attach() ) {
        return addrs;
    }

    VIRTADDR start = read_addr(addr);
    VIRTADDR end = read_addr(addr + 4);
    int bytes = end - start;
    int entries = bytes / 4;
    TRACE << "enumerating vector at" << hex << addr << "START" << start << "END" << end << "UNVERIFIED ENTRIES" << dec << entries;
    VIRTADDR tmp_addr = 0;

    if (entries > 5000) {
        LOGW << "vector at" << hexify(addr) << "has over 5000 entries! (" << entries << ")";
    }

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
    Q_UNUSED(addr);
    Q_UNUSED(str);
    return 0;
}

int DFInstanceOSX::write_int(const VIRTADDR &addr, const int &val) {
    return write_raw(addr, sizeof(int), (void*)&val);
}

uint DFInstanceOSX::calculate_checksum() {
    // ELF binaries don't seem to store a linker timestamp, so just MD5 the file.
    uint md5 = 0; // we're going to throw away a lot of this checksum we just need 4bytes worth
    QProcess *proc = new QProcess(this);
    QStringList args;
    args << "-q";
    args << m_loc_of_dfexe;
    proc->start("md5", args);
    if (proc->waitForReadyRead(3000)) {
        QByteArray out = proc->readAll();
        QString str_md5(out);
        QStringList chunks = str_md5.split(" ");
        str_md5 = chunks[0];
        bool ok;
        md5 = str_md5.mid(0, 8).toUInt(&ok,16); // use the first 8 bytes
        TRACE << "GOT MD5:" << md5;
    }
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

int DFInstanceOSX::read_raw(const VIRTADDR &addr, int bytes, QByteArray &buffer) {
    kern_return_t result;

    vm_size_t readsize = 0;

    result = vm_read_overwrite(m_task, addr, bytes,
                               (vm_address_t)buffer.data(),
                               &readsize );

    if ( result != KERN_SUCCESS ) {
        //LOGW << "Unable to read " << bytes << " byte(s) from " << hexify(addr) <<
        //"into buffer" << &buffer << endl;
        return -1;
    }

    return readsize;
}

int DFInstanceOSX::write_raw(const VIRTADDR &addr, const int &bytes, void *buffer) {
    kern_return_t result;

    result = vm_write( m_task,  addr,  (vm_offset_t)buffer,  bytes );
    if ( result != KERN_SUCCESS ) {
        //LOGW << "Unable to write " << bytes << " byte(s) from " << hexify(addr) <<
        //"into buffer" << &buffer << endl;
        return -1;
    }

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
            LOGD << "Found running copy, pid:" << m_pid << "path:" << m_loc_of_dfexe;
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
    m_layout = get_memory_layout(hexify(calculate_checksum()).toLower(), !connect_anyway);

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


