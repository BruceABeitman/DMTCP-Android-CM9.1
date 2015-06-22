/*
 * Command line access to services.
 *
 */
 
#include <binder/Parcel.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <utils/TextOutput.h>

#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

using namespace android;

void writeString16(Parcel& parcel, const char* string)
{
    if (string != NULL)
    {
        parcel.writeString16(String16(string));
    }
    else
    {
        parcel.writeInt32(-1);
    }
}

// get the name of the generic interface we hold a reference to
static String16 get_interface_name(sp<IBinder> service)
{
    if (service != NULL) {
        Parcel data, reply;
        status_t err = service->transact(IBinder::INTERFACE_TRANSACTION, data, &reply);
        if (err == NO_ERROR) {
            return reply.readString16();
        }
    }
    return String16();
}

static String8 good_old_string(const String16& src)
{
    String8 name8;
    char ch8[2];
    ch8[1] = 0;
    for (unsigned j = 0; j < src.size(); j++) {
        char16_t ch = src[j];
        if (ch < 128) ch8[0] = (char)ch;
        name8.append(ch8);
    }
    return name8;
}

int main(int argc, char* const argv[])
{
    sp<IServiceManager> sm = defaultServiceManager();
    fflush(stdout);
    if (sm == NULL) {
        aerr << "service: Unable to get default service manager!" << endl;
        return 20;
    }
    
    bool wantsUsage = false;
    int result = 0;
    while (1) {
        Vector<String16> services = sm->listServices();
        aout << "Found " << services.size() << " services:" << endl;
        for (unsigned i = 0; i < services.size(); i++) {
            String16 name = services[i];
            sp<IBinder> service = sm->checkService(name);
            aout << i 
                 << "\t" << good_old_string(name) 
                 << ": [" << good_old_string(get_interface_name(service)) << "]"
                 << endl;
        }
        sleep(3);
    }
    
    return 0;
}

