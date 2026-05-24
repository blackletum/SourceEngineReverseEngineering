#include "extension.h"
#include "hang_watchdog.h"

#include <execinfo.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

static pthread_t g_async_thread;
static pthread_t g_main_thread;
static bool g_async_thread_started = false;
static bool g_stack_handler_installed = false;
static struct sigaction g_previous_stack_action;
static volatile time_t g_main_thread_last_heartbeat = 0;

static const int kMainThreadHangSeconds = 15;
static const int kWatchdogCheckIntervalSeconds = 1;
static const char* kStackTraceOutputPath = "/tmp/server_utils_main_thread_hang.trace";
static const off_t kMaxStackTraceFileBytes = 100 * 1024 * 1024;

void TouchMainThreadHeartbeat()
{
    g_main_thread_last_heartbeat = time(NULL);
}

static void MainThreadStackTraceSignalHandler(int sig)
{
    if (sig != SIGUSR2)
    {
        return;
    }

    int file = open(kStackTraceOutputPath, O_RDWR | O_CREAT | O_APPEND, 0644);
    if (file < 0)
    {
        return;
    }

    struct stat file_info;
    if (fstat(file, &file_info) == 0 && file_info.st_size >= kMaxStackTraceFileBytes)
    {
        if (ftruncate(file, 0) == 0)
        {
            lseek(file, 0, SEEK_SET);
        }
    }

    void* frames[128];
    int frame_count = backtrace(frames, 128);
    if (frame_count > 0)
    {
        static const char header[] = "=== main thread stack trace (watchdog) ===\n";
        write(file, header, sizeof(header) - 1);
        backtrace_symbols_fd(frames, frame_count, file);
    }

    close(file);
}

static void InstallMainThreadStackHandler()
{
    if (g_stack_handler_installed)
    {
        return;
    }

    struct sigaction action;
    action.sa_handler = MainThreadStackTraceSignalHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_RESTART;

    if (sigaction(SIGUSR2, &action, &g_previous_stack_action) != 0)
    {
        rootconsole->ConsolePrint("Failed to install SIGUSR2 stack handler");
        return;
    }

    g_stack_handler_installed = true;
}

static void UninstallMainThreadStackHandler()
{
    if (!g_stack_handler_installed)
    {
        return;
    }

    sigaction(SIGUSR2, &g_previous_stack_action, NULL);
    g_stack_handler_installed = false;
}

static void* AsyncWorkerMain(void* data)
{
    (void)data;

    bool hang_reported = false;

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    for (;;)
    {
        time_t now = time(NULL);
        time_t last = g_main_thread_last_heartbeat;

        if (last != 0 && (now - last) >= kMainThreadHangSeconds)
        {
            if (!hang_reported)
            {
                rootconsole->ConsolePrint("Watchdog: main thread unresponsive for %d seconds, dumping stack trace to %s", kMainThreadHangSeconds, kStackTraceOutputPath);
                pthread_kill(g_main_thread, SIGUSR2);
                hang_reported = true;
            }
        }
        else
        {
            hang_reported = false;
        }

        sleep(kWatchdogCheckIntervalSeconds);
    }

    return NULL;
}

void InitHangWatchdog()
{
    if (g_async_thread_started)
    {
        return;
    }

    g_main_thread = pthread_self();
    TouchMainThreadHeartbeat();
    InstallMainThreadStackHandler();

    int create_result = pthread_create(&g_async_thread, NULL, &AsyncWorkerMain, NULL);
    if (create_result != 0)
    {
        rootconsole->ConsolePrint("Failed to create async worker thread: %d", create_result);
        UninstallMainThreadStackHandler();
        return;
    }

    int detach_result = pthread_detach(g_async_thread);
    if (detach_result != 0)
    {
        rootconsole->ConsolePrint("Failed to detach async worker thread: %d", detach_result);
        pthread_cancel(g_async_thread);
        UninstallMainThreadStackHandler();
        return;
    }

    g_async_thread_started = true;
    rootconsole->ConsolePrint("Started detached async worker thread");
}

void DeinitHangWatchdog()
{
    if (g_async_thread_started)
    {
        int cancel_result = pthread_cancel(g_async_thread);
        if (cancel_result != 0)
        {
            rootconsole->ConsolePrint("Failed to cancel async worker thread: %d", cancel_result);
        }

        g_async_thread_started = false;
    }

    UninstallMainThreadStackHandler();
}
