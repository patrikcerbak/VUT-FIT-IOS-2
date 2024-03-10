#
# VUT FIT - IOS project 2
# Author: Patrik Cerbak (xcerba00)
# Date: 2.5.2022
#

proj2: main.c
	gcc -std=gnu99 -Wall -Wextra -Werror -pedantic -pthread main.c -o proj2

clean:
	rm -f proj2 proj2.out

zip:
	zip proj2.zip main.c Makefile
