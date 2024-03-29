#include <fcntl.h>
#include <linux/uinput.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <syslog.h>
#include <unistd.h>

void emit(int fd, int type, int code, int val) {
  struct input_event ie;

  ie.type = type;
  ie.code = code;
  ie.value = val;
  /* timestamp values below are ignored */
  ie.time.tv_sec = 0;
  ie.time.tv_usec = 0;

  int written = write(fd, &ie, sizeof(ie)) != sizeof(ie);
  if (written != sizeof(ie)) {
    // syslog(LOG_lERR, "partial write, %d vs %llu!", written, sizeof(ie));
    // exit(1);
  }
}

enum Options {
  None = 0x0,
  Sleep = 0x1,
  Log = 0x2,
  SleepAndLog = Sleep | Log,
};

enum Modifiers {
  NoMods = 0x00,
  LeftShift = 0x01,
  RightShift = 0x02,
  LeftAlt = 0x04,
  RightAlt = 0x08,
  LeftCtrl = 0x10,
  RightCtrl = 0x20,
  Meta = 0x40,
};

void press(int fd, int code, Modifiers modifiers = Modifiers::NoMods,
           Options options = Options::Log) {
  if (modifiers & Modifiers::LeftShift) {
    press(fd, KEY_LEFTSHIFT, Modifiers::NoMods, Options::None);
  }
  if (modifiers & Modifiers::RightShift) {
    press(fd, KEY_RIGHTSHIFT, Modifiers::NoMods, Options::None);
  }
  if (modifiers & Modifiers::LeftAlt) {
    press(fd, KEY_LEFTALT, Modifiers::NoMods, Options::None);
  }
  if (modifiers & Modifiers::RightAlt) {
    press(fd, KEY_RIGHTALT, Modifiers::NoMods, Options::None);
  }
  if (modifiers & Modifiers::LeftCtrl) {
    press(fd, KEY_LEFTCTRL, Modifiers::NoMods, Options::None);
  }
  if (modifiers & Modifiers::RightCtrl) {
    press(fd, KEY_RIGHTCTRL, Modifiers::NoMods, Options::None);
  }
  if (modifiers & Modifiers::Meta) {
    press(fd, KEY_OPTION, Modifiers::NoMods, Options::None);
  }

  emit(fd, EV_KEY, code, 1);
  emit(fd, EV_SYN, SYN_REPORT, 0);
  if (options & Options::Log) {
    syslog(LOG_DEBUG, "pressed %d", code);
  }
  if (options & Options::Sleep) {
    usleep(200 * 1000);
  }
}

void release(int fd, int code, Modifiers modifiers = Modifiers::NoMods,
             Options options = Options::Log) {
  emit(fd, EV_KEY, code, 0);
  emit(fd, EV_SYN, SYN_REPORT, 0);

  if (modifiers & Modifiers::LeftShift) {
    release(fd, KEY_LEFTSHIFT, Modifiers::NoMods, Options::None);
  }
  if (modifiers & Modifiers::RightShift) {
    release(fd, KEY_RIGHTSHIFT, Modifiers::NoMods, Options::None);
  }
  if (modifiers & Modifiers::LeftAlt) {
    release(fd, KEY_LEFTALT, Modifiers::NoMods, Options::None);
  }
  if (modifiers & Modifiers::RightAlt) {
    release(fd, KEY_RIGHTALT, Modifiers::NoMods, Options::None);
  }
  if (modifiers & Modifiers::LeftCtrl) {
    release(fd, KEY_LEFTCTRL, Modifiers::NoMods, Options::None);
  }
  if (modifiers & Modifiers::RightCtrl) {
    release(fd, KEY_RIGHTCTRL, Modifiers::NoMods, Options::None);
  }
  if (modifiers & Modifiers::Meta) {
    release(fd, KEY_OPTION, Modifiers::NoMods, Options::None);
  }

  if (options & Options::Log) {
    syslog(LOG_DEBUG, "released %d", code);
  }
  if (options & Options::Sleep) {
    usleep(200 * 1000);
  }
}

void click(int fd, int code, Modifiers modifiers = Modifiers::NoMods,
           Options options = Options::Log) {
  press(fd, code, modifiers);
  release(fd, code, modifiers);

  if (options & Options::Log) {
    syslog(LOG_DEBUG, "clicked %d", code);
  }
  if (options & Options::Sleep) {
    usleep(200 * 1000);
  }
}

void writeSentence(int fd) {
  click(fd, KEY_F);
  click(fd, KEY_R);
  click(fd, KEY_O);
  click(fd, KEY_M);
  click(fd, KEY_SPACE);
  click(fd, KEY_T);
  click(fd, KEY_H);
  click(fd, KEY_E);
  click(fd, KEY_SPACE);
  click(fd, KEY_G);
  click(fd, KEY_H);
  click(fd, KEY_O);
  click(fd, KEY_S);
  click(fd, KEY_T);
}

int main(void) {
  struct uinput_setup usetup;

  int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
  if (fd < 0) {
    perror("open");
    exit(1);
  }

  /*
   * The ioctls below will enable the device that is about to be
   * created, to pass key events, in this case the space key.
   */
  if (ioctl(fd, UI_SET_EVBIT, EV_KEY) < 0) {
    perror("EV_KEY");
    exit(1);
  }
  if (ioctl(fd, UI_SET_EVBIT, EV_SYN) < 0) {
    perror("EV_KEY");
    exit(1);
  }

  auto supportKey = [](int fd, int code) {
    if (ioctl(fd, UI_SET_KEYBIT, code) < 0) {
      perror("KEY_SPACE");
      exit(1);
    }
  };

  supportKey(fd, KEY_SPACE);
  supportKey(fd, KEY_OPTION);
  supportKey(fd, KEY_LEFTCTRL);
  supportKey(fd, KEY_RIGHTCTRL);
  supportKey(fd, KEY_LEFTALT);
  supportKey(fd, KEY_RIGHTALT);
  supportKey(fd, KEY_LEFTSHIFT);
  supportKey(fd, KEY_RIGHTSHIFT);
  supportKey(fd, KEY_SEMICOLON);
  supportKey(fd, KEY_COMMA);
  supportKey(fd, KEY_DOT);
  supportKey(fd, KEY_SLASH);
  supportKey(fd, KEY_APOSTROPHE);
  supportKey(fd, KEY_GRAVE);
  supportKey(fd, KEY_EQUAL);
  supportKey(fd, KEY_A);
  supportKey(fd, KEY_B);
  supportKey(fd, KEY_C);
  supportKey(fd, KEY_D);
  supportKey(fd, KEY_E);
  supportKey(fd, KEY_F);
  supportKey(fd, KEY_G);
  supportKey(fd, KEY_H);
  supportKey(fd, KEY_I);
  supportKey(fd, KEY_J);
  supportKey(fd, KEY_K);
  supportKey(fd, KEY_L);
  supportKey(fd, KEY_M);
  supportKey(fd, KEY_N);
  supportKey(fd, KEY_O);
  supportKey(fd, KEY_P);
  supportKey(fd, KEY_Q);
  supportKey(fd, KEY_R);
  supportKey(fd, KEY_S);
  supportKey(fd, KEY_T);
  supportKey(fd, KEY_U);
  supportKey(fd, KEY_V);
  supportKey(fd, KEY_W);
  supportKey(fd, KEY_X);
  supportKey(fd, KEY_Y);
  supportKey(fd, KEY_Z);
  supportKey(fd, KEY_1);
  supportKey(fd, KEY_2);
  supportKey(fd, KEY_3);
  supportKey(fd, KEY_4);
  supportKey(fd, KEY_5);
  supportKey(fd, KEY_6);
  supportKey(fd, KEY_7);
  supportKey(fd, KEY_8);
  supportKey(fd, KEY_9);
  supportKey(fd, KEY_0);

  memset(&usetup, 0, sizeof(usetup));
  usetup.id.bustype = BUS_USB;
  usetup.id.vendor = 0x1234;  /* sample vendor */
  usetup.id.product = 0x5678; /* sample product */
  strcpy(usetup.name, "Example device");

  if (ioctl(fd, UI_DEV_SETUP, &usetup) < 0) {
    perror("usetup");
    exit(1);
  }
  if (ioctl(fd, UI_DEV_CREATE) < 0) {
    perror("UI_DEV_CREATE");
    exit(1);
  }

  /*
   * On UI_DEV_CREATE the kernel will create the device node for this
   * device. We are inserting a pause here so that userspace has time
   * to detect, initialize the new device, and can start listening to
   * the event, otherwise it will not notice the event we are about
   * to send. This pause is only needed in our example code!
   */
  syslog(LOG_DEBUG, "created, sleeping...");
  sleep(1);
  syslog(LOG_DEBUG, "awake...");

  // +?``
  // click(fd, KEY_EQUAL, Modifiers::NoMods);
  // click(fd, KEY_EQUAL, Modifiers::LeftShift);
  // click(fd, KEY_EQUAL, Modifiers::LeftAlt);
  // click(fd, KEY_EQUAL, Modifiers(Modifiers::LeftAlt | Modifiers::LeftShift));
  // usleep(200 * 1000);
  press(fd, KEY_GRAVE, Modifiers(Modifiers::LeftAlt | Modifiers::LeftShift));
  click(fd, KEY_U);
  release(fd, KEY_GRAVE, Modifiers(Modifiers::LeftAlt | Modifiers::LeftShift));

  // // 1!!''
  // click(fd, KEY_1, Modifiers::NoMods);
  // click(fd, KEY_1, Modifiers::LeftShift);
  // click(fd, KEY_1, Modifiers::RightShift);
  // click(fd, KEY_1, Modifiers::LeftAlt);
  // click(fd, KEY_1, Modifiers::RightAlt);
  // usleep(200 * 1000);

  // // 2""@@
  // click(fd, KEY_2, Modifiers::NoMods);
  // click(fd, KEY_2, Modifiers::LeftShift);
  // click(fd, KEY_2, Modifiers::RightShift);
  // click(fd, KEY_2, Modifiers::LeftAlt);
  // click(fd, KEY_2, Modifiers::RightAlt);
  // usleep(200 * 1000);

  // // 3##
  // click(fd, KEY_3, Modifiers::NoMods);
  // click(fd, KEY_3, Modifiers::LeftShift);
  // click(fd, KEY_3, Modifiers::RightShift);
  // click(fd, KEY_3, Modifiers::LeftAlt);
  // click(fd, KEY_3, Modifiers::RightAlt);
  // usleep(200 * 1000);

  // // 4$$
  // click(fd, KEY_4, Modifiers::NoMods);
  // click(fd, KEY_4, Modifiers::LeftShift);
  // click(fd, KEY_4, Modifiers::RightShift);
  // click(fd, KEY_4, Modifiers::LeftAlt);
  // click(fd, KEY_4, Modifiers::RightAlt);
  // usleep(200 * 1000);

  // // 5%%
  // click(fd, KEY_5, Modifiers::NoMods);
  // click(fd, KEY_5, Modifiers::LeftShift);
  // click(fd, KEY_5, Modifiers::RightShift);
  // click(fd, KEY_5, Modifiers::LeftAlt);
  // click(fd, KEY_5, Modifiers::RightAlt);
  // usleep(200 * 1000);

  // // 6&&
  // click(fd, KEY_6, Modifiers::NoMods);
  // click(fd, KEY_6, Modifiers::LeftShift);
  // click(fd, KEY_6, Modifiers::RightShift);
  // click(fd, KEY_6, Modifiers::LeftAlt);
  // click(fd, KEY_6, Modifiers::RightAlt);
  // usleep(200 * 1000);

  // // 7//
  // click(fd, KEY_7, Modifiers::NoMods);
  // click(fd, KEY_7, Modifiers::LeftShift);
  // click(fd, KEY_7, Modifiers::RightShift);
  // click(fd, KEY_7, Modifiers::LeftAlt);
  // click(fd, KEY_7, Modifiers::RightAlt);
  // usleep(200 * 1000);

  // // 8((
  // click(fd, KEY_8, Modifiers::NoMods);
  // click(fd, KEY_8, Modifiers::LeftShift);
  // click(fd, KEY_8, Modifiers::RightShift);
  // click(fd, KEY_8, Modifiers::LeftAlt);
  // click(fd, KEY_8, Modifiers::RightAlt);
  // usleep(200 * 1000);

  // // 9))
  // click(fd, KEY_9, Modifiers::NoMods);
  // click(fd, KEY_9, Modifiers::LeftShift);
  // click(fd, KEY_9, Modifiers::RightShift);
  // click(fd, KEY_9, Modifiers::LeftAlt);
  // click(fd, KEY_9, Modifiers::RightAlt);
  // usleep(200 * 1000);

  // // 0==
  // click(fd, KEY_0, Modifiers::NoMods);
  // click(fd, KEY_0, Modifiers::LeftShift);
  // click(fd, KEY_0, Modifiers::RightShift);
  // click(fd, KEY_0, Modifiers::LeftAlt);
  // click(fd, KEY_0, Modifiers::RightAlt);
  // usleep(200 * 1000);

  // writeSentence(fd);
  // click(fd, KEY_A, Modifiers::NoMods, Options::SleepAndLog);

  // press(fd, KEY_OPTION);
  // press(fd, KEY_A);
  // release(fd, KEY_OPTION);
  // release(fd, KEY_A, Options::SleepAndLog);

  /*
   * Give userspace some time to read the events before we destroy the
   * device with UI_DEV_DESTROY.
   */
  syslog(LOG_DEBUG, "all written, sleeping...");
  sleep(2);
  syslog(LOG_DEBUG, "awake, destroying...");

  if (ioctl(fd, UI_DEV_DESTROY) < 0) {
    perror("UI_DEV_DESTROY");
    exit(1);
  }

  if (close(fd) < 0) {
    perror("close");
    exit(1);
  }

  return 0;
}
