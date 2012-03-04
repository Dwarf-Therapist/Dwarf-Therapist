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

#include "dfinstance.h"
#include "dfinstanceosx.h"
#include "defines.h"
#include "dwarf.h"
#include "utils.h"
#include "gamedatareader.h"
#include "memorylayout.h"
#include "cp437codec.h"
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
}

QVector<uint> DFInstanceOSX::enumerate_vector(const uint &addr) {
    attach();
    QVector<uint> addrs;
    detach();
    return addrs;
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

QString DFInstanceOSX::read_string(const uint &addr) {
	QString ret_val = "FOO";
	return ret_val;
}

int DFInstanceOSX::write_string(const uint &addr, const QString &str) {
    Q_UNUSED(addr);
    Q_UNUSED(str);
	return 0;
}

int DFInstanceOSX::write_int(const uint &addr, const int &val) {
    return 0;
}

bool DFInstanceOSX::attach() {
    return true;
}

bool DFInstanceOSX::detach() {
	return true;
}

int DFInstanceOSX::read_raw(const VIRTADDR &addr, int bytes, QByteArray &buffer) {
    return 0;
}

int DFInstanceOSX::write_raw(const VIRTADDR &addr, const int &bytes, void *buffer) {
    return 0;
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
    m_layout = get_memory_layout(hexify(calculate_checksum()).toLower(), !connect_anyway);

    map_virtual_memory();

    [authPool release];

    return true;
}

void DFInstanceOSX::map_virtual_memory() {
    if (m_regions.isEmpty()) {
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
    }
}

bool DFInstance::authorize() {
    // Create authorization reference
    OSStatus status;
    AuthorizationRef authorizationRef;

    if( DFInstanceOSX::isAuthorized() ) {
         return true;
    }

    char therapistExe[1024];
    uint32_t size = sizeof(therapistExe);
    _NSGetExecutablePath(therapistExe, &size);
    printf("Therapist path: %s\n", therapistExe);

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

