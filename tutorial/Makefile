# $Id: Makefile,v 1.2 2007/09/28 13:29:25 raimc Exp $ 
# PLEASE CHANGE THE FLAGS BELOW TO YOUR INSTALLATION PATHS
CCFLAGS  = -g -O2 -I/scr/fremdling/raim/include
LDFLAGS  = -L/scr/fremdling/raim/lib -lODES -lsbml \
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
