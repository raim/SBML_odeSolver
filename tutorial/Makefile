# $Id: Makefile,v 1.1 2007/09/28 13:14:16 raimc Exp $ 
# PLEASE CHANGE THE FLAGS BELOW TO YOUR INSTALLATION PATHS
CCFLAGS  = -g -O2 -I/your/local/path/include
LDFLAGS  = -L/your/local/path/lib -lODES -lsbml \
           -lsundials_cvodes -lsundials_nvecserial \
           -ldl -lm -lstdc++
# rules
all: integrate odemodel integratorinstance setvariablevalue
integrate:
	gcc $(CCFLAGS) integrate.c  $(LDFLAGS) -o integrate 
odemodel:
	gcc $(CCFLAGS) odemodel.c  $(LDFLAGS) -o odemodel  
integratorinstance:
	gcc $(CCFLAGS) integratorinstance.c  $(LDFLAGS) -o integratorinstance 
setvariablevalue:
	gcc $(CCFLAGS) setvariablevalue.c  $(LDFLAGS) -o setvariablevalue
clean:
	rm -f setvariablevalue integratorinstance odemodel integrate
# End of file
