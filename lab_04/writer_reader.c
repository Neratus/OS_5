#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#define WRITER_COUNT 5
#define READER_COUNT 4

HANDLE can_read;
HANDLE can_write;
HANDLE bmutex;

LONG read_cnt = 0;
LONG write_cnt = 0;
// LONG active_reader = 0;

char symbol = 'a';

volatile int cnt = 0;
volatile LONG flag = 0;

BOOL signal(DWORD signal) {
  flag = 1;
  return TRUE;
}

void start_read() {
  read_cnt++;
  if (write_cnt || WaitForSingleObject(can_write, 0))
    WaitForSingleObject(can_read, INFINITE);
  SetEvent(can_read);
  WaitForSingleObject(bmutex, INFINITE);
  // active_reader++;
  read_cnt--;
  ReleaseMutex(bmutex);
}

void stop_read() {
  // active_reader--;
  // if (active_reader == 0)
  SetEvent(can_write);
}

void start_write() {
  write_cnt++;
  if (WaitForSingleObject(can_read, 0))
    WaitForSingleObject(can_write, INFINITE);
  write_cnt--;
}

void stop_write() {
  ResetEvent(can_write);
  if (write_cnt > 0)
    SetEvent(can_write);
  else
    SetEvent(can_read);
}

unsigned writer(PVOID param) {
  while (flag == 0) {
    start_write();

    if (symbol == 'z') {
      cnt++;
      symbol = 'a';
      if (cnt >= 3) {
        GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, 0);
        break;
      }
    } else {
      symbol++;
    }
    printf("%ld write %c\n", GetCurrentThreadId(), symbol);
    stop_write();
  }
  return 0;
}

unsigned reader(PVOID param) {
  while (flag == 0) {
    start_read();
    printf("%ld read %c\n", GetCurrentThreadId(), symbol);
    stop_read();
  }
  return 0;
}

int main() {
  if (!SetConsoleCtrlHandler(signal, TRUE)) {
    printf("handler error\n");
    ExitProcess(1);
  }

  unsigned thread_ids[WRITER_COUNT + READER_COUNT];
  HANDLE threads[WRITER_COUNT + READER_COUNT];

  if ((can_read = CreateEvent(NULL, FALSE, TRUE, NULL)) == NULL) {
    printf("event error\n");
    ExitProcess(1);
  }

  if ((can_write = CreateEvent(NULL, TRUE, TRUE, NULL)) == NULL) {
    printf("event error\n");
    ExitProcess(1);
  }

  if ((bmutex = CreateMutex(NULL, FALSE, NULL)) == NULL) {
    printf("mutex error\n");
    ExitProcess(1);
  }

  for (int i = 0; i < WRITER_COUNT + READER_COUNT; i++) {
    if (i % 2 == 0) {
      threads[i] =
          (HANDLE)_beginthreadex(NULL, 0, writer, NULL, 0, thread_ids + i);
    } else {
      threads[i] =
          (HANDLE)_beginthreadex(NULL, 0, reader, NULL, 0, thread_ids + i);
    }
    if (threads[i] == NULL) {
      printf("thread error\n");
      ExitProcess(1);
    }
  }

  unsigned wres =
      WaitForMultipleObjects(WRITER_COUNT + READER_COUNT, threads, TRUE, 3000);
  if (wres == WAIT_OBJECT_0) {
    printf("All threads finished successfully\n");
  } else if (wres == WAIT_TIMEOUT) {
    printf("Timeout: some threads didn't finish in time\n");
    for (int i = 0; i < WRITER_COUNT + READER_COUNT; i++) {
      TerminateThread(threads[i], 0);
    }
  } else {
    printf("Wait error: %ld\n", GetLastError());
  }

  for (int i = 0; i < WRITER_COUNT + READER_COUNT; i++) {
    CloseHandle(threads[i]);
  }

  CloseHandle(can_read);
  CloseHandle(can_write);
  CloseHandle(bmutex);

  return 0;
}
