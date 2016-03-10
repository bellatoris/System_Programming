
machine3.o:     file format elf32-i386


Disassembly of section .text:

00000000 <.text>:
   0:	89 e8                	mov    %ebp,%eax
   2:	01 28                	add    %ebp,(%eax)
   4:	89 45 00             	mov    %eax,0x0(%ebp)
   7:	83 28 30             	subl   $0x30,(%eax)
   a:	83 6d 00 30          	subl   $0x30,0x0(%ebp)
   e:	89 e5                	mov    %esp,%ebp
  10:	89 2c 24             	mov    %ebp,(%esp)
  13:	89 65 00             	mov    %esp,0x0(%ebp)
  16:	bd 00 2f 68 55       	mov    $0x55682f00,%ebp
  1b:	89 e5                	mov    %esp,%ebp
  1d:	83 c5 2c             	add    $0x2c,%ebp
