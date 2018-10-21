#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define TAG_REQUEST       10
#define TAG_BOTTLE        20
#define TAG_TURN          30
#define TAG_FINISHED      99

#define MSG_DEFAULT       0

#define BOTTLE_ONE        1
#define BOTTLE_TWO        2
#define BOTTLE_THREE      4

#define BOTTLE_TURN       128

#define REQUEST_ITERATIONS 10

int m_qty_proccess = 5;
int m_rank;

int m_access_counter;
int m_finished;

int m_thirsty;
int m_holds_bottle[5];
int m_holds_turn[5];
int m_owes_bottle[5];
int m_needs_bottle[5];
int x;
int y;

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
    m_thirsty = 0;
    for (i = 0; i < m_qty_proccess; i++)
        m_holds_bottle[i] = 0;
    for (i = 0; i < m_qty_proccess; i++)
        m_holds_turn[i] = 0;
    for (i = 0; i < m_qty_proccess; i++)
        m_owes_bottle[i] = 0;
    for (i = 0; i < m_qty_proccess; i++)
        m_needs_bottle[i] = 0;
    x = 0;
    y = 0;
    if (m_rank == 0)
    {
        m_holds_turn[4] = 1;
        m_holds_bottle[4] = BOTTLE_ONE | BOTTLE_TWO | BOTTLE_THREE;
        /*m_needs_bottle[4] = BOTTLE_ONE | BOTTLE_TWO | BOTTLE_THREE;
        m_needs_bottle[1] = BOTTLE_ONE | BOTTLE_TWO | BOTTLE_THREE;*/
        m_needs_bottle[4] = rand() % 8;
        m_needs_bottle[1] = rand() % 8;
    }
    else if (m_rank == 1)
    {
        m_holds_turn[0] = 1;
        m_holds_turn[2] = 1;
        m_holds_bottle[0] = BOTTLE_ONE | BOTTLE_TWO | BOTTLE_THREE;
        /*m_needs_bottle[0] = BOTTLE_ONE | BOTTLE_TWO | BOTTLE_THREE;
        m_needs_bottle[2] = BOTTLE_ONE | BOTTLE_TWO | BOTTLE_THREE;*/
        m_needs_bottle[0] = rand() % 8;
        m_needs_bottle[2] = rand() % 8;
    }
    else if (m_rank == 2)
    {
        m_holds_turn[3] = 1;
        m_holds_bottle[1] = BOTTLE_ONE | BOTTLE_TWO | BOTTLE_THREE;;
        /*m_needs_bottle[1] = BOTTLE_ONE | BOTTLE_TWO | BOTTLE_THREE;
        m_needs_bottle[3] = BOTTLE_ONE | BOTTLE_TWO | BOTTLE_THREE;*/
        m_needs_bottle[1] = rand() % 8;
        m_needs_bottle[3] = rand() % 8;
    }
    else if (m_rank == 3)
    {
        m_holds_turn[4] = 1;
        m_holds_bottle[2] = BOTTLE_ONE | BOTTLE_TWO | BOTTLE_THREE;;
        /*m_needs_bottle[2] = BOTTLE_ONE | BOTTLE_TWO | BOTTLE_THREE;
        m_needs_bottle[4] = BOTTLE_ONE | BOTTLE_TWO | BOTTLE_THREE;*/
        m_needs_bottle[2] = rand() % 8;
        m_needs_bottle[4] = rand() % 8;
    }
    else if (m_rank == 4)
    {
        m_holds_bottle[3] = BOTTLE_ONE | BOTTLE_TWO | BOTTLE_THREE;;
        /*m_needs_bottle[3] = BOTTLE_ONE | BOTTLE_TWO | BOTTLE_THREE;
        m_needs_bottle[0] = BOTTLE_ONE | BOTTLE_TWO | BOTTLE_THREE;*/
        m_needs_bottle[3] = rand() % 8;
        m_needs_bottle[0] = rand() % 8;
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
    m_thirsty = 1;
    for (i = 0; i < m_qty_proccess; i++)
        if (m_neighbors[m_rank][i] == 1)
            if ((m_needs_bottle[i] & m_holds_bottle[i]) != m_needs_bottle[i])
            {
                int inner_msg[2];
                x = m_needs_bottle[i] - (m_needs_bottle[i] & m_holds_bottle[i]);
                inner_msg[0] = x;
                inner_msg[1] = MSG_DEFAULT;
                MPI_Send(&inner_msg, 2, MPI_INT, i, TAG_REQUEST, MPI_COMM_WORLD);
                x = 0;
            }
}

void make_requests()
{
    send_request();
}

int holds_all_bottles()
{
    int i;
    for (i = 0; i < m_qty_proccess; i++)
        if (m_neighbors[m_rank][i] == 1)
            if ((m_needs_bottle[i] & m_holds_bottle[i]) != m_needs_bottle[i])
                return 0;
    return 1;
}

void receive_request(int msg[], int origin)
{
    int i = BOTTLE_ONE;
    printf("%d:Receiving REQUEST from [%d].\n", m_rank, origin);
    while (i < 8)
    {
        if (msg[0] & i)
        {
            if (!m_thirsty || !m_holds_turn[origin] || !(m_needs_bottle[origin] & i))
            {
                if (m_holds_bottle[origin] & i)
                    m_holds_bottle[origin] -= i;
                x |= i;
                if (m_thirsty && (m_needs_bottle[origin] & i))
                    y |= i;
            }
            else
            {
                m_owes_bottle[origin] |= i;
            }
        }
        i *= 2;
    }
    if (x)
    {
        if (!y)
        {
            int inner_msg[2];
            inner_msg[0] = x;
            inner_msg[1] = MSG_DEFAULT;
            MPI_Send(&inner_msg, 2, MPI_INT, origin, TAG_BOTTLE, MPI_COMM_WORLD);
        }
        else
        {
            int inner_msg[2];
            inner_msg[0] = x;
            inner_msg[1] = y;
            MPI_Send(&inner_msg, 2, MPI_INT, origin, TAG_BOTTLE, MPI_COMM_WORLD);
            y = 0;
        }
        x = 0;
    }
}

void receive_bottle(int msg[], int origin)
{
    int i;
    int holds_all_needed_bottles = 1;
    printf("%d:Receiving BOTTLE from [%d].\n", m_rank, origin);
    m_holds_bottle[origin] |= msg[0];
    if (msg[1] & BOTTLE_TURN) /* t = turn */
        m_holds_turn[origin] = 1;
    else /* t = request(y) */
        m_owes_bottle[origin] |= msg[1];
    for (i = 0; i < m_qty_proccess; ++i)
        if (m_neighbors[m_rank][i] == 1)
            if ((m_needs_bottle[i] & m_holds_bottle[i]) != m_needs_bottle[i])
                holds_all_needed_bottles = 0;
    if (holds_all_needed_bottles)
    {
        access_shared_resources();
        m_thirsty = 0;
        for (i = 0; i < m_qty_proccess; ++i)
            if (m_neighbors[m_rank][i] == 1)
            {
                if (m_holds_turn[i])
                {
                    int b = BOTTLE_ONE;
                    m_holds_turn[i] = 0;
                    while (b < 8)
                    {
                        if ((m_holds_bottle[i] & b) && (m_owes_bottle[i] & b))
                        {
                            m_owes_bottle[i] -= b;
                            m_holds_bottle[i] -= b;
                            x |= b;
                        }
                        b *= 2;
                    }
                    if (x)
                    {
                        int inner_msg[2];
                        inner_msg[0] = x;
                        inner_msg[1] = BOTTLE_TURN;
                        MPI_Send(&inner_msg, 1, MPI_INT, i, TAG_BOTTLE, MPI_COMM_WORLD);
                        x = 0;
                    }
                    else
                    {
                        int inner_msg[2];
                        inner_msg[0] = MSG_DEFAULT;
                        inner_msg[1] = BOTTLE_TURN;
                        MPI_Send(&inner_msg, 1, MPI_INT, i, TAG_TURN, MPI_COMM_WORLD);
                    }
                }
            }
    }
}

void receive_turn(int msg[], int origin)
{
    printf("%d:Receiving TURN from [%d].\n", m_rank, origin);
    m_holds_turn[origin] = 1;
}

void receive_finished(int msg[], int origin)
{
    printf("%d:Receiving FINISHED from [%d].\n", m_rank, origin);
    m_finished++;
}

void handle_messages()
{
    int flag = -1;
    int j;
    int origin;
    int sent_finished_msg = 0;
    int tag;
    int msg[2];
    MPI_Request request;
    MPI_Status status;
    /* for (j = 0; j < (2*m_qty_proccess)-(2*1); j++) */
    while (m_access_counter < REQUEST_ITERATIONS || m_finished < m_qty_proccess-1)
    {
        if (m_access_counter < REQUEST_ITERATIONS && !(rand() % 1000000))
        {
            if (!holds_all_bottles())
                send_request();
            else
                access_shared_resources();
        }
        if (flag)
        {
            MPI_Irecv(&msg, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &request);
            flag = 0;
        }
        MPI_Test(&request, &flag, &status);
        if (flag)
        {
            /* MPI_Recv(&msg, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status); */
            origin = status.MPI_SOURCE;
            tag = status.MPI_TAG;
            // printf("%d:{Origin[%d], Tag[%d]}\n", m_rank, origin, tag);
            if (tag == TAG_REQUEST)
                receive_request(msg, origin);
            else if (tag == TAG_BOTTLE)
                receive_bottle(msg, origin);
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
    make_requests();
    handle_messages();

    /* Finalizing MPI */
    printf("%d:Finalizing proccess.\n", m_rank);
    fflush(stdout);
    MPI_Finalize();
}
