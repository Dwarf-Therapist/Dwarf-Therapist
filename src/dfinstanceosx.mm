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
#include <QMessageBox>

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

#include "dfinstance.h"
#include "dfinstanceosx.h"
#include "defines.h"
#include "dwarf.h"
#include "utils.h"
#include "gamedatareader.h"
#include "memorylayout.h"
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
    : DFInstanceNix(parent)
    , m_alloc_start(0)
    , m_alloc_end(0)
    , m_ruid(getuid())
{
    // Try to reacquire root privileges, don't check success,
    // authorize() will handle that
    seteuid(0);

    // This makes the unauthorized DT instance quit at the proper time.
    // No, fflush(stdout) does not work.
    puts("");
    if(!authorize()) {
        exit(1);
    }

    // temporarily drop privileges
    if (-1 == seteuid(m_ruid)) {
        int err = errno;
        LOGE << "seteuid failed:" << strerror(err);
        return;
    }
}

DFInstanceOSX::~DFInstanceOSX() {
    if(m_attach_count > 0) {
        detach();
    }
}

bool DFInstanceOSX::attach() {
    kern_return_t result;
    if(m_attach_count > 0) {
        m_attach_count++;
        return true;
    }

    // reacquire dropped privileges
    if (-1 == seteuid(0)) {
        int err = errno;
        LOGE << "seteuid(0) failed:" << strerror(err);
        return false;
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

    m_attach_count--;
    if( m_attach_count > 0 ) {
        return true;
    }

    result = task_resume(m_task);
    if ( result != KERN_SUCCESS ) {
        return false;
    }

    // temporarily drop privileges
    if (-1 == seteuid(m_ruid)) {
        int err = errno;
        LOGE << "seteuid failed:" << strerror(err);
        return false;
    }
    return true;
}

USIZE DFInstanceOSX::read_raw(VIRTADDR addr, USIZE bytes, void *buffer) {
    vm_size_t bytes_read = 0;
    memset(buffer, 0, bytes);

    // try to attach, will be ignored if we're already attached
    attach();
    vm_read_overwrite(m_task, (vm_address_t)addr, bytes, (vm_address_t)buffer, static_cast<vm_size_t*>(&bytes_read));
    detach();
    return bytes_read;
}

USIZE DFInstanceOSX::write_raw(VIRTADDR addr, USIZE bytes, const void *buffer) {
    attach();
    kern_return_t result = vm_write(m_task, (vm_address_t)addr, (pointer_t)buffer, bytes);
    detach();
    return result == KERN_SUCCESS ? bytes : 0;
}

bool DFInstanceOSX::set_pid(){
    // reacquire dropped privileges
    if (-1 == seteuid(0)) {
        int err = errno;
        LOGE << "seteuid failed:" << strerror(err);
        return false;
    }

    NSAutoreleasePool *authPool = [[NSAutoreleasePool alloc] init];

    NSWorkspace *workspace = [NSWorkspace sharedWorkspace];
    NSArray *launchedApps = [workspace runningApplications];

    unsigned i, len = [launchedApps count];

    // compile process array
    for ( i = 0; i < len; i++ ) {
        NSRunningApplication *application = [launchedApps objectAtIndex:i];
        if ( [[application localizedName] isEqualToString:@"dwarfort.exe" ]) {

            m_loc_of_dfexe = QString( [[[application executableURL] path]UTF8String]);
            m_pid = [application processIdentifier];
            LOGI << "Found running copy, pid:" << m_pid << "path:" << m_loc_of_dfexe;
        }
    }

    kern_return_t kret = task_for_pid( current_task(), m_pid, &m_task );
    [authPool release];

    // temporarily drop privileges
    if (-1 == seteuid(m_ruid)) {
        int err = errno;
        LOGE << "seteuid failed:" << strerror(err);
        return false;
    }

    if (m_pid == 0 || ! (kret == KERN_SUCCESS)) {
        LOGE << "can't find running copy";
        return false;
    }

    return true;
}

void DFInstanceOSX::find_running_copy() {
    m_status = DFS_DISCONNECTED;

    if(!set_pid()){
        return;
    }

    m_status = DFS_CONNECTED;
    set_memory_layout(calculate_checksum());
    m_alloc_start = 0;
    m_alloc_end = 0;
    m_pointer_size = sizeof(VIRTADDR); // TODO: set pointer size
}

VIRTADDR DFInstanceOSX::alloc_chunk(USIZE size) {
    if (size > 1048576) {
        return 0;
    }

    if ((m_alloc_end - m_alloc_start) < size) {
        int apages = (size * 2 + 4095)/4096;
        int asize = apages * 4096;

        vm_address_t new_block;

        kern_return_t err;

        if (m_alloc_start == 0) {
            err = vm_allocate( m_task, &new_block, asize, VM_FLAGS_ANYWHERE );
        } else {
            new_block = m_alloc_end;
            err = vm_allocate( m_task, &new_block, asize, VM_FLAGS_FIXED );
        }

        if (err != KERN_SUCCESS) {
            return 0;
        }

        if (new_block != m_alloc_end) {
            m_alloc_start = new_block;
        }
        m_alloc_end = new_block + asize;
    }

    VIRTADDR rv = m_alloc_start;
    m_alloc_start += size;

    return rv;
}

bool DFInstanceOSX::authorize() {
    // Create authorization reference
    OSStatus status;
    AuthorizationRef authorizationRef;

    char therapistExe[1024];
    uint32_t size = sizeof(therapistExe);
    _NSGetExecutablePath(therapistExe, &size);
    //NSLog(@"Therapist path: %s\n", therapistExe);

    if( DFInstanceOSX::isAuthorized() ) {
        // Ignore SIGPIPE.
        signal(SIGPIPE, SIG_IGN);
        // Ensure we're in the correct path
        QDir dir(therapistExe);
        dir.makeAbsolute();
        dir.cdUp();
        chdir(dir.absolutePath().toLocal8Bit());
        fflush(stdout); // Often causes SIGPIPE.

        // Authorization for remote memory access on OS X trips Qt's setuid detection
        #if defined(Q_OS_MAC) && QT_VERSION >= QT_VERSION_CHECK(5,3,0)
        QCoreApplication::setSetuidAllowed(true);
        #endif
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
    char *args[] = {NULL};

    status = AuthorizationExecuteWithPrivileges(authorizationRef, therapistExe,
                                                kAuthorizationFlagDefaults, args, &pipe);
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
    NSDictionary *applicationAttributes = [[NSFileManager defaultManager] attributesOfItemAtPath:[[[NSBundle mainBundle] executablePath] stringByResolvingSymlinksInPath] error:nil];
    return ([applicationAttributes filePosixPermissions] == 1517 && [[applicationAttributes fileGroupOwnerAccountName] isEqualToString: @"procmod"]);
    [authPool release];
}

int DFInstanceOSX::VM_TYPE_OFFSET(){
    if(m_pointer_size == 8)
        return 0x1; // For 64bit
    return 0x4;     // For 32bit
}
