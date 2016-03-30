movl %ebp,%eax

addl %ebp,(%eax)
movl %eax,(%ebp)
sub $0x30,(%eax)
sub $0x30,(%ebp)
mov %esp,%ebp
mov %ebp,(%esp)
mov %esp,(%ebp)
movl $0x55682f00,%ebp
movl %esp,%ebp
addl $0x2c,%ebp
