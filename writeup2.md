# Boot into a root shell (system maintainance)

1. Hold `Shift` key as the VM if booting up.
2. Enter the following line which will the shell to run
```
live init=/bin/bash
```
3. Once done, press `Enter` key to save the changes.
This causes the system to boot the kernel and run `/bin/bash` instead of `/sbin/init`

http://nadir.is.online.fr/download/admin/Linux-1/01-Admin%20Linux%20seance%201-HOWTO-reset%20du%20password%20root%20avec%20grub%20v2.pdf
https://unix.stackexchange.com/questions/105278/resetting-password-of-another-linux
https://ubuntuforums.org/showthread.php?t=989517
