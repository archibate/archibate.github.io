x:
	clang++ -std=c++20 -stdlib=libc++ -fimplicit-modules -fimplicit-module-maps -fprebuilt-module-path=. -Xclang -emit-module-interface -c -o helloworld.pcm helloworld.cpp
	clang++ -std=c++20 -stdlib=libc++ -fimplicit-modules -fimplicit-module-maps -fprebuilt-module-path=. -c -o helloworld.o helloworld.cpp
	clang++ -std=c++20 -stdlib=libc++ -fimplicit-modules -fimplicit-module-maps -fprebuilt-module-path=. -c -o main.o main.cpp
	clang++ -std=c++20 -stdlib=libc++ -fimplicit-modules -fimplicit-module-maps -fprebuilt-module-path=. -o a.out main.o helloworld.o
	./a.out
