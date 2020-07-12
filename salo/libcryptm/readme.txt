PLEASE READ BEFORE FIRST SALO UPSTART!

I) libcryptm currently supporting only 64-bit (!) and only RHEL-based (!!) linux distributions. 


II) If salo doesent start with "not found" error like this:

	[muxamed666@localhost binary]$ ./salobin
	./salobin: error while loading shared libraries: libcryptm.so: cannot open shared object file: No such file or directory

Fix:
1) Switch to root!
2) Copy libcryptm.so from this folder to /lib64, for example: cp ../libcryptm/libcryptm.so /lib64/libcryptm.so
3) Switch back to your user
4) Check if problem solved.

If not, check if file mode, and SELinux allowes system (/bin/ld in particular) and "salobin" process to read file.  


III) If salo doesent start whith "not found" error, but whith other lib, not "libcryptm", or if it launches and instantly segfaults:

After previous fixes, you may (or not) run into a trouble whith other lib dependencies.
Commonly, libs comes to linux distribution in a packages, but libcryptm - not. So if it doesent work,
you need to work as "humanized" package manager - resolve dependecies with your own hands.

libcryptm needs OpenSSL packages (for system-independent hashing), and libpthread (multitheading support).
Both this packages comes with Fedora, CentOS and other Rad Hat systems by default, but since there is no
package manager control over this lib, not guaranteed that all libs are installed.

Firstly, check OpenSSL, libpthread, GNU GCC, GCC C++ and SELinux packages are installed and updated. 
If so, but problem not solved, you need to start your own investigation!

1) Use "ldd" command to see what dependencies are broken. You will see list of libs, if it have path and address its ok, if not 
you found the broken dependecy.
2) Run  ldd both for ./salobin file, and for /lib64/libcryptm.so file.
	# ldd salobin
	# ldd /lib64/libcryptm.so
3) if ldd salobin doesent founds libcryptm.so, goto previous problem fix
4) If you found other libs are broken, google its full package name for your distro:
	For example: you will found "openssl-devel" for libssl.so in Fedora 23.
5) Install missing packages by your package manager
	For example: sudo dnf install openssl-devel in Fedora 23.
6) Check if problem solved.
