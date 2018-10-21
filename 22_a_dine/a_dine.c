#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define TAG_REQUEST       10
#define TAG_FORK          20
#define TAG_TURN          30
#define TAG_FINISHED      99

#define MSG_DEFAULT       0
#define FORK_REQUEST      1
#define FORK_TURN         2

#define REQUEST_ITERATIONS 10

int m_qty_proccess = 5;
int m_rank;

int m_access_counter;
int m_finished;

int m_hungry;
int m_holds_fork[5];
int m_holds_turn[5];
int m_owes_fork[5];

int m_neighbors[5][5] = {
                            {0, 1, 0, 0, 1},
                            {1, 0, 1, 0, 0},
                            {0, 1, 0, 1, 0},
                            {0, 0, 1, 0, 1},
                            {1, 0, 0, 1, 0}
                        };

void init_variables()
{
    int i;
    m_hungry = 0;
    for (i = 0; i < m_qty_proccess; i++)
        m_holds_fork[i] = 0;
    for (i = 0; i < m_qty_proccess; i++)
        m_holds_turn[i] = 0;
    for (i = 0; i < m_qty_proccess; i++)
        m_owes_fork[i] = 0;
    if (m_rank == 0)
    {
        m_holds_turn[4] = 1;
        m_holds_fork[4] = 1;
    }
    else if (m_rank == 1)
    {
        m_holds_turn[0] = 1;
        m_holds_turn[2] = 1;
        m_holds_fork[0] = 1;
    }
    else if (m_rank == 2)
    {
        m_holds_turn[3] = 1;
        m_holds_fork[1] = 1;
    }
    else if (m_rank == 3)
    {
        m_holds_turn[4] = 1;
        m_holds_fork[2] = 1;
    }
    else if (m_rank == 4)
    {
        m_holds_fork[3] = 1;
    }
}

void access_shared_resources()
{
    printf("%d:Accessing resources.\n", m_rank);
    m_access_counter++;
}

void send_request()
{
    int i;
    m_hungry = 1;
    for (i = 0; i < m_qty_proccess; i++)
        if (m_neighbors[m_rank][i] == 1)
            if (!m_holds_fork[i])
            {
                int msg = 0;
                MPI_Send(&msg, 1, MPI_INT, i, TAG_REQUEST, MPI_COMM_WORLD);
            }
}

void receive_request(int msg, int origin)
{
    printf("%d:Receiving REQUEST from [%d].\n", m_rank, origin);
    if (!m_hungry || !m_holds_turn[origin])
    {
        m_holds_fork[origin] = 0;
        if (!m_hungry)
        {
            int msg = MSG_DEFAULT;
            MPI_Send(&msg, 1, MPI_INT, origin, TAG_FORK, MPI_COMM_WORLD);
        }
        else
        {
            int msg = FORK_REQUEST;
            MPI_Send(&msg, 1, MPI_INT, origin, TAG_FORK, MPI_COMM_WORLD);
        }
    }
    else
    {
        m_owes_fork[origin] = 1;
    }
}

int holds_all_forks()
{
    int i;
    int holds_all_forks = 1;
    for (i = 0; i < m_qty_proccess; ++i)
        if (m_neighbors[m_rank][i] == 1)
            if (!m_holds_fork[i])
                holds_all_forks = 0;
    return holds_all_forks;
}

void receive_fork(int msg, int origin)
{
    int i;
    /*int holds_all_forks = 1;*/
    printf("%d:Receiving FORK from [%d].\n", m_rank, origin);
    m_holds_fork[origin] = 1;
    if (msg == FORK_TURN)
        m_holds_turn[origin] = 1;
    if (msg == FORK_REQUEST)
        m_owes_fork[origin] = 1;
    /*for (i = 0; i < m_qty_proccess; ++i)
        if (m_neighbors[m_rank][i] == 1)
            if (!m_holds_fork[i])
                holds_all_forks = 0;*/
    /*if (holds_all_forks)*/
    if (holds_all_forks())
    {
        access_shared_resources();
        m_hungry = 0;
        for (i = 0; i < m_qty_proccess; ++i)
            if (m_neighbors[m_rank][i] == 1)
            {
                if (m_holds_turn[i])
                {
                    m_holds_turn[i] = 0;
                    if (m_owes_fork[i])
                    {
                        int msg = FORK_TURN;
                        m_owes_fork[i] = 0;
                        m_holds_fork[i] = 0;
                        MPI_Send(&msg, 1, MPI_INT, i, TAG_FORK, MPI_COMM_WORLD);
                    }
                    else
                    {
                        int msg = MSG_DEFAULT;
                        MPI_Send(&msg, 1, MPI_INT, i, TAG_TURN, MPI_COMM_WORLD);
                    }
                }
            }
    }
}

void receive_turn(int msg, int origin)
{
    printf("%d:Receiving TURN from [%d].\n", m_rank, origin);
    m_holds_turn[origin] = 1;
}

void receive_finished(int msg, int origin)
{
    printf("%d:Receiving FINISHED from [%d].\n", m_rank, origin);
    m_finished++;
}

void handle_messages()
{
    int flag = -1;
    int j;
    int msg;
    int origin;
    int sent_finished_msg = 0;
    int tag;
    MPI_Request request;
    MPI_Status status;
    /* for (j = 0; j < (2*m_qty_proccess)-(2*1); j++) */
    /* while (1) */
    while (m_access_counter < REQUEST_ITERATIONS || m_finished < m_qty_proccess-1)
    {
        if (m_access_counter < REQUEST_ITERATIONS && !(rand() % 1000000))
        {
            if (!holds_all_forks())
                send_request();
            else
                access_shared_resources();
        }
        if (flag)
        {
            MPI_Irecv(&msg, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &request);
            flag = 0;
        }
        MPI_Test(&request, &flag, &status);
        if (flag)
        {
            origin = status.MPI_SOURCE;
            tag = status.MPI_TAG;
            // printf("%d:{Origin[%d], Tag[%d]}\n", m_rank, origin, tag);
            if (tag == TAG_REQUEST)
                receive_request(msg, origin);
            else if (tag == TAG_FORK)
                receive_fork(msg, origin);
            else if (tag == TAG_TURN)
                receive_turn(msg, origin);
            else if (tag == TAG_FINISHED)
                receive_finished(msg, origin);
            flag = -1;
        }
        if (m_access_counter >= REQUEST_ITERATIONS && !sent_finished_msg)
        {
            int i;
            for (i = 0; i < m_qty_proccess; i++)
                if (m_rank != i)
                {
                    int inner_msg = MSG_DEFAULT;
                    printf("%d:Sending FINISHED to [%d].\n", m_rank, i);
                    MPI_Send(&inner_msg, 1, MPI_INT, i, TAG_FINISHED, MPI_COMM_WORLD);
                }
            sent_finished_msg = 1;
        }
    }
}

int main(int argc, char** argv)
{
    /* Initializing MPI */
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &m_rank);

    init_variables();
    /*make_requests();*/
    handle_messages();

    /* Finalizing MPI */
    printf("%d:Finalizing proccess.\n", m_rank);
    fflush(stdout);
    MPI_Finalize();
}
