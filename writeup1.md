# Explore ISO file
Mount ISO file
```sh
┌──$ [~/42/2022/boot2root]
└─>  hdiutil mount BornToSecHackMe-v1.1.iso
/dev/disk2               /Volumes/BornToSec
```

Start explore ISO file before installing a VM.
```sh
┌──$ [~/42/2022/boot2root]
└─>  cd /Volumes/BornToSec
┌──$ [/Volumes/BornToSec]
└─>  tree -a
.
├── .disk
│   ├── base_installable
│   ├── cd_type
│   ├── info
│   └── release_notes_url
├── README.diskdefines              # file which may be referenced during installation
├── casper                          # folder containing the read-only part of the file system
│   ├── README.diskdefines          # file which may be referenced during installation
│   ├── filesystem.manifest         # file which contains information about the installation packages
│   ├── filesystem.manifest-desktop # file which contains information about the installation packages
│   ├── filesystem.size             # the required disk space for OS is usually twice the amount of bytes found in this file
│   ├── filesystem.squashfs         # read-only file system which is especially used to create Live CD
│   ├── initrd.gz                   # Gzip compressed data which provides the capability to load a RAM disk by the boot loader
│   └── vmlinuz                     # Linux kernel x86 boot executable
├── install
│   └── memtest
├── isolinux                        # the folder containing the loader isolinux.bin. It will search for the config file directory in the order /boot/isolinux, /isolinux, /
│   ├── boot.cat                    # file which is of no interest except it is required
│   ├── isolinux.bin                # a boot loader for ISO9660 (file system for CD-ROM disks)
│   ├── isolinux.cfg                # configuration file which contains directives settings for the color and the structure of the menu
│   ├── splash.png                  # screen image shown at boot
│   └── vesamenu.c32                # the program which displays the boot menu
├── md5sum.txt                      # fingerprint (checksum) of disk files
├── preseed                         # preseed allows to automate an installer by pre-specifying answers to installation questions
│   └── custom.seed                 # preconfiguration file
└── ubuntu

5 directories, 21 files
```











# Install ISO

Change the default networking mode of VM from NAT to Bridge to have more interaction between host machine and VM.
In bridged networking mode, the default gateway of VM is home router.
So, after start, VM will get the IP address in the same network range.
```sh
┌──$ [~/42/2022/boot2root]
└─>  VBoxManage modifyvm "boot2root" --nic1 bridged
```

Run the VM.
```sh
┌──$ [~/42/2022/boot2root]
└─>  VBoxManage startvm "boot2root" --type headless
Waiting for VM "boot2root" to power on...
VM "boot2root" has been successfully started.
```

Find the subnet address and the mask on which actuall device is running.
```sh
┌──$ [~/42/2022/boot2root]
└─>  ifconfig en0
en0: flags=8863<UP,BROADCAST,SMART,RUNNING,SIMPLEX,MULTICAST> mtu 1500
	ether 4c:32:75:9a:8f:4f
	inet6 fe80::c8e:b7dd:85d9:5032%en0 prefixlen 64 secured scopeid 0x7
	inet 192.168.0.13 netmask 0xffffff00 broadcast 192.168.0.255
	nd6 options=201<PERFORMNUD,DAD>
	media: autoselect
	status: active
```

```
0xffffff00 -> 255.255.255.0
192.168.0.13 & 255.255.255.0 = 192.168.0.0/24
```

Run a scan with `-sn` option to perform only host discovery without port scanning (aka "pign scan")
```sh
┌──$ [~/42/2022/boot2root]
└─>  sudo nmap -sn 192.168.0.0/24
Password: ******************
Starting Nmap 7.91 ( https://nmap.org ) at 2022-02-18 17:44 CET
[...]
Nmap scan report for 192.168.0.21                              # boot2root IP address
Host is up (0.00035s latency).                                 # boot2root VM status
MAC Address: 08:00:27:AC:88:E4 (Oracle VirtualBox virtual NIC) # boot2root MAC address
-
Nmap scan report for 192.168.0.13                              # my host
Host is up.                                                    # my host status
Nmap done: 256 IP addresses (6 hosts up) scanned in 23.57 seconds
```


Once IP address is found, run a scan with  `-p-`, `-sV` and `-O` options to scan ports from 1 through 65535, enable service/version detection to determine what application is running and enable OS detection. Aggressive scan may be performed using option `-A`.
```sh
┌──$ [~/42/2022/boot2root]
└─>  sudo nmap -p- -sV -O 192.168.0.21
Password: ******************
Starting Nmap 7.92 ( https://nmap.org ) at 2022-03-15 09:55 EDT
Nmap scan report for 192.168.0.21
Host is up (0.00048s latency).
Not shown: 65529 closed tcp ports (reset)
PORT    STATE SERVICE  VERSION
21/tcp  open  ftp      vsftpd 2.0.8 or later
22/tcp  open  ssh      OpenSSH 5.9p1 Debian 5ubuntu1.7 (Ubuntu Linux; protocol 2.0)
80/tcp  open  http     Apache httpd 2.2.22 ((Ubuntu))
143/tcp open  imap     Dovecot imapd
443/tcp open  ssl/http Apache httpd 2.2.22
993/tcp open  ssl/imap Dovecot imapd
MAC Address: 08:00:27:FD:21:F4 (Oracle VirtualBox virtual NIC)
Device type: general purpose
Running: Linux 3.X
OS CPE: cpe:/o:linux:linux_kernel:3
OS details: Linux 3.2 - 3.10, Linux 3.2 - 3.16
Network Distance: 1 hop
Service Info: Host: 127.0.1.1; OS: Linux; CPE: cpe:/o:linux:linux_kernel

OS and Service detection performed. Please report any incorrect results at https://nmap.org/submit/ .
Nmap done: 1 IP address (1 host up) scanned in 21.01 seconds
```
The scan reveals that a few serices are running and their corresponding ports.

Website on port `80` has title "Hack me if you can". Source code doesn't reveal anything interesting.

Trying access random page, show taht server is running on Apache/2.2.22
```
http://192.168.0.21/admin
Not Found

The requested URL /admin was not found on this server.
Apache/2.2.22 (Ubuntu) Server at 192.168.0.21 Port 80
```

Instead of manual/random tries to find a valid webpage, a [path traversal attack](https://owasp.org/www-community/attacks/Path_Traversal) can be performed.
`dirb` is a web content scanner which will looks for existing web objects. It basically works by launching a dictionary based attack.

Run `dirb` on port 80 reveal that only `index.html` and `fonts/` are accessible.
```shell
┌──(kali㉿kali)-[~]
└─$ dirb http://192.168.0.21

-----------------
DIRB v2.22
By The Dark Raver
-----------------

START_TIME: Tue Mar 15 10:28:02 2022
URL_BASE: http://192.168.0.21/
WORDLIST_FILES: /usr/share/dirb/wordlists/common.txt

-----------------

GENERATED WORDS: 4612

---- Scanning URL: http://192.168.0.21/ ----
+ http://192.168.0.21/cgi-bin/ (CODE:403|SIZE:288)
==> DIRECTORY: http://192.168.0.21/fonts/
+ http://192.168.0.21/forum (CODE:403|SIZE:285)
+ http://192.168.0.21/index.html (CODE:200|SIZE:1025)
+ http://192.168.0.21/server-status (CODE:403|SIZE:293)

---- Entering directory: http://192.168.0.21/fonts/ ----
(!) WARNING: Directory IS LISTABLE. No need to scan it.
    (Use mode '-w' if you want to scan it anyway)

-----------------
END_TIME: Tue Mar 15 10:28:13 2022
DOWNLOADED: 4612 - FOUND: 4
```

Running same scan, with `-r` option, on HTTPS port 443, reveal `forum`, `phpmyadmin` and `webmail` accessible.
```shell
┌──(kali㉿kali)-[~]
└─$ dirb https://192.168.0.21:443 /usr/share/dirb/wordlists/common.txt -r

-----------------
DIRB v2.22
By The Dark Raver
-----------------

START_TIME: Tue Mar 15 10:35:54 2022
URL_BASE: https://192.168.0.21:443/
WORDLIST_FILES: /usr/share/dirb/wordlists/common.txt
OPTION: Not Recursive

-----------------

GENERATED WORDS: 4612

---- Scanning URL: https://192.168.0.21:443/ ----
+ https://192.168.0.21:443/cgi-bin/ (CODE:403|SIZE:289)
==> DIRECTORY: https://192.168.0.21:443/forum/
==> DIRECTORY: https://192.168.0.21:443/phpmyadmin/
+ https://192.168.0.21:443/server-status (CODE:403|SIZE:294)
==> DIRECTORY: https://192.168.0.21:443/webmail/

-----------------
END_TIME: Tue Mar 15 10:36:13 2022
DOWNLOADED: 4612 - FOUND: 2
```

The forum on https://192.168.0.21:443/forum/, is claming in the title *HackMe*.

First, there is no way to create a new user, only to log in with existing one: `admin`, `lmezard`, `qudevide`, `thor`, `wandre` and `zaz`.

A post (Probleme login ?)[https://192.168.0.21/forum/index.php?id=6] contains extract of `auth.log` file. `/var/log/auth.log` keeps authentication logs for successful or failed logins, and authentication processes.

Copy this log into a file and sort by users which tried to authentificate.
```shell
┌──$ [/tmp]
└─>  cat auth.log |  grep -o "user .* from" | sort -u
user !q\]Ej?*5K5cy*AJ from
user PlcmSpIp from
user adam from
user adm from
user admin from
user ftpuser from
user guest from
user nagios from
user naos from
user nvdb from
user pi from
user support from
user test from
user ubnt from
user user from
```
Only three users were succesefuly authenticated.
```shell
┌──$ [/tmp]
└─>  cat auth.log |  grep -o "session opened for user .*"  | sort -u
session opened for user admin by (uid=0)
session opened for user lmezard by (uid=1040)
session opened for user root by (uid=0)
session opened for user root by admin(uid=0)
session opened for user root by admin(uid=1000)
```
It seems clearly that user which tried to authentificate misstyped password instead of username `!q\]Ej?*5K5cy*AJ`. As succesefuly authenticated users are determined, try to log in forum under of one of those users.

The password allowed to authentificate on the (forum)[https://192.168.0.21/forum] as `lmezard` user, but not on (webmail)[https://192.168.0.21/webmail] and (phpmyadmin)[https://192.168.0.21/phpmyadmin/].