# $Id: Makefile,v 1.3 2007/09/29 05:07:16 raimc Exp $ 
# PLEASE CHANGE THE FLAGS BELOW TO YOUR INSTALLATION PATHS
CCFLAGS  = -g -O2 -I/usr/local/include
LDFLAGS  = -L/usr/local/lib -lODES -lsbml \
           -lsundials_cvodes -lsundials_nvecserial \
           -ldl -lm -lstdc++
# rules
all: integrate odemodel integratorinstance setvariablevalue evaluateast
integrate: integrate.c
	gcc $(CCFLAGS) integrate.c  $(LDFLAGS) -o integrate 
odemodel: odemodel.c
	gcc $(CCFLAGS) odemodel.c  $(LDFLAGS) -o odemodel  
integratorinstance: integratorinstance.c
	gcc $(CCFLAGS) integratorinstance.c  $(LDFLAGS) -o integratorinstance 
setvariablevalue: setvariablevalue.c 
	gcc $(CCFLAGS) setvariablevalue.c  $(LDFLAGS) -o setvariablevalue
evaluateast: evaluateast.c
	gcc $(CCFLAGS) evaluateast.c  $(LDFLAGS) -o evaluateast
clean:
	rm -f setvariablevalue integratorinstance odemodel integrate evaluateast
# End of file
