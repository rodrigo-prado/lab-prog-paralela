# MACROS

MPICC   = /usr/lib64/openmpi/bin/mpicc

# DEPENDENCIES

hello_world.exe: prop_sem_real.c
	$(MPICC) prop_sem_real.c -o prop_sem_real.exe 
	@echo Compilado com Sucesso!

clean:
	rm -f core prop_sem_real.exe 
