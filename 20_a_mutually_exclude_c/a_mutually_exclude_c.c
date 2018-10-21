#include <stdio.h>
#include <mpi.h>
#include <pthread.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define TAG_REQUEST  10
#define TAG_RESPONSE 20
#define TAG_FINISHED 99

#define MSG_DEFAULT  0

#define REQUEST_ITERATIONS 10

void send_request();

int m_qty_proccess = 6;
int m_rank;

int m_access_counter;
int m_finished;

int m_seq;
int m_highest_seq;
int m_expected;
int m_owes_reply[6];

int m_neighbors[6][6] = {
                            {0, 1, 1, 1, 1, 1},
                            {1, 0, 1, 1, 1, 1},
                            {1, 1, 0, 1, 1, 1},
                            {1, 1, 1, 0, 1, 1},
                            {1, 1, 1, 1, 0, 1},
                            {1, 1, 1, 1, 1, 0}
                        };

void init_variables()
{
    int i;
    printf("%d:Initializing variables.\n", m_rank);
    m_highest_seq = 0;
    m_expected = 0;
    for (i = 0; i < m_qty_proccess; i++)
        m_owes_reply[i] = 0;
    m_access_counter = 0;
    m_finished = 0;
}

void access_shared_resource()
{
    int i;
    printf("%d:Accessing resource.\n", m_rank);
    m_access_counter++;
}

void send_request()
{
    int i;
    m_seq = m_highest_seq + 1;
    m_expected = m_qty_proccess - 1;
    for (i = 0; i < m_qty_proccess; i++)
        if (m_neighbors[m_rank][i] == 1)
            MPI_Send(&m_seq, 1, MPI_INT, i, TAG_REQUEST, MPI_COMM_WORLD);
}

void make_requests()
{
    send_request();
}

void receive_request(int seq, int origin)
{
    m_highest_seq = MAX(m_highest_seq, seq);
    if (m_expected == 0 || seq < m_seq || (seq == m_seq && origin < m_rank))
        MPI_Send(&seq, 1, MPI_INT, origin, TAG_RESPONSE, MPI_COMM_WORLD);
    else
        m_owes_reply[origin] = 1;
}

void receive_response(int seq, int origin)
{
    int i;
    m_expected = m_expected - 1;
    if (m_expected == 0)
    {
        access_shared_resource();
        for (i = 0; i < m_qty_proccess; i++)
            if (m_neighbors[m_rank][i] == 1)
                if (m_owes_reply[i])
                {
                    m_owes_reply[i] = 0;
                    MPI_Send(&m_seq, 1, MPI_INT, i, TAG_RESPONSE, MPI_COMM_WORLD);
                }
        if (m_access_counter < REQUEST_ITERATIONS)
            send_request();
        else
        {
            for (i = 0; i < m_qty_proccess; i++)
                if (m_neighbors[m_rank][i] == 1)
                {
                    int inner_msg = MSG_DEFAULT;
                    MPI_Send(&inner_msg, 1, MPI_INT, i, TAG_FINISHED, MPI_COMM_WORLD);
                }
        }
    }
}

void receive_finished(int seq, int origin)
{
    m_finished++;
}

void handle_messages()
{
    int j;
    int origin;
    int tag;
    int seq;
    MPI_Status status;
    /* for (j = 0; j < (2*m_qty_proccess)-(2*1); j++) */
    while (m_access_counter < REQUEST_ITERATIONS || m_finished < m_qty_proccess-1)
    {
        MPI_Recv(&seq, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        origin = status.MPI_SOURCE;
        tag = status.MPI_TAG;
        if (tag == TAG_REQUEST)
            receive_request(seq, origin);
        else if (tag == TAG_RESPONSE)
            receive_response(seq, origin);
        else if (tag == TAG_FINISHED)
            receive_finished(seq, origin);
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

