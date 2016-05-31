cc  =  gcc
lib = -fPIC  -lm -g -Wall `pkg-config --libs --cflags glib-2.0`
clib = -fPIC -g -Wall -lm

main=cJSON.o  main.o   myeEchoManage.o  myeManage.o      myeProcessPool.o  myeThreadPool.o   myeUtil.o myeHttpManage.o  myeContral.o
http=../maoYeHTTP/myeApplication.o ../maoYeHTTP/myeHttpConnect.o ../maoYeHTTP/myeHttpUtil.o

all:pool
pool:$(main) $(http)
	$(cc) -o pool $(main) $(http)  $(lib) 

$(main):%.o:%.c
	$(cc) -c $< -o $@  $(lib)
$(http):../maoYeHTTP/%.o:../maoYeHTTP/%.c
	$(cc) -c $< -o $@  $(lib)


.PHONY: clean
clean:
	-rm *.o 
	-rm	pool 
	-rm ../maoYeHTTP/*.o



