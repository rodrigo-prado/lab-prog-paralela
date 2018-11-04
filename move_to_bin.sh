#!/bin/bash

HOME_DIR=./
DEST_DIR=./test/bin
PROJ_DIR=30_2-mbs-grasp-sequential/
cp -f $HOME_DIR/$PROJ_DIR/dmer $DEST_DIR/dmer_Sequential
PROJ_DIR=31_2-mbs-grasp-parallel-mpi
cp -f $HOME_DIR/$PROJ_DIR/dmer $DEST_DIR/dmer_MPI_LB
PROJ_DIR=32_2-mbs-grasp-parallel-mpi
cp -f $HOME_DIR/$PROJ_DIR/dmer $DEST_DIR/dmer_MPI+Pthread_LB
PROJ_DIR=33_2-mbs-grasp-parallel-mpi
cp -f $HOME_DIR/$PROJ_DIR/dmer $DEST_DIR/dmer_MPI+OpenMP_LB
PROJ_DIR=41_2-mbs-grasp-parallel-mpi
cp -f $HOME_DIR/$PROJ_DIR/dmer $DEST_DIR/dmer_MPI
PROJ_DIR=42_2-mbs-grasp-parallel-mpi
cp -f $HOME_DIR/$PROJ_DIR/dmer $DEST_DIR/dmer_MPI+Pthread
PROJ_DIR=43_2-mbs-grasp-parallel-mpi
cp -f $HOME_DIR/$PROJ_DIR/dmer $DEST_DIR/dmer_MPI+OpenMP
