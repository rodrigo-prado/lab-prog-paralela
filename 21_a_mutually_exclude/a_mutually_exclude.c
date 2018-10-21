#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define TAG_REQUEST     10
#define TAG_GRANTED     20
#define TAG_PROBE       30
#define TAG_DELAYED     40
#define TAG_RELEASE     50
#define TAG_RELINQUISH  60
#define TAG_FINISHED    99

#define MSG_DEFAULT  0
#define REQUEST_ITERATIONS 3

struct node
{
    int seq;
    int id;
};

void send_request();

int m_qty_proccess = 6;
int m_rank;

int m_access_counter;
int m_finished;

int m_seq;
int m_highest_seq;
int m_expected;
int m_locked;
int m_granted_node;
int m_granted_seq;
int m_granted_id;
int m_has_probed;
int m_got_probe[6];
int m_got_delayed[6];

struct node queue[6+1]; 
int queue_pointer = 0;

int m_sets[6][6] = {
                    {0, 1, 1, 1, 0, 0},
                    {0, 0, 1, 1, 1, 0},
                    {1, 0, 0, 1, 0, 1},
                    {1, 0, 1, 0, 0, 0},
                    {0, 1, 1, 1, 0, 0},
                    {0, 0, 1, 1, 1, 0}
                   };

int m_neighbors[6][6] = {
                            {0, 1, 1, 1, 0, 0},
                            {1, 0, 1, 1, 1, 0},
                            {1, 1, 0, 1, 1, 1},
                            {1, 1, 1, 0, 1, 1},
                            {0, 1, 1, 1, 0, 1},
                            {0, 0, 1, 1, 1, 0}
                        };

/* +++ Priority Queue +++ */
// Implementacao baseada no livro Estruturas de Dados e Seus algoritmos de
// Jayme Luiz Szwarcfiter e Lilian Markenzon

void troca(int i, int j)
{
    int aux_seq;
    int aux_id;
    aux_seq = queue[j].seq;
    aux_id = queue[j].id;
    queue[j] = queue[i];
    queue[j].seq = queue[i].seq;
    queue[j].id = queue[i].id;
    queue[i].seq = aux_seq;
    queue[i].id = aux_id;
}

void subir(int i)
{
    int j = i/2;
    if (j >= 1)
        if (queue[i].seq < queue[j].seq)
        {   
            troca(i, j);
            subir(j);
        }
        else if (queue[i].seq == queue[j].seq && queue[i].id < queue[j].id)
        {
            troca(i, j);
            subir(j);
        }
}

void descer(int i, int n)
{
    int j = 2 * i;
    if (j <= n)
    {
        if (j < n)
        {
            if (queue[j+1].seq < queue[j].seq)
                j = j + 1;
            else if (queue[j+1].seq == queue[j].seq && queue[j+1].id < queue[j].id)
                j = j + 1;
        }
        if (queue[i].seq > queue[j].seq)
        {
            troca(i, j);
            descer(j, n);
        }
        else if (queue[i].seq == queue[j].seq && queue[i].id > queue[j].id)
        {
            troca(i, j);
            descer(j, n);
        }
    }
}

void insercao(int seq, int id)
{
    if (queue_pointer < 6)
    {
        ++queue_pointer;
        queue[queue_pointer].seq = seq;
        queue[queue_pointer].id = id;
        subir(queue_pointer);
    }
    else
    {
        printf("%d:Queue overflow.\n", m_rank);
        exit(1);
    }
}

int remocao()
{
    if (queue_pointer != 0)
    {
        int retorno = queue[1].id;
        queue[1].seq = queue[queue_pointer].seq;
        queue[1].id = queue[queue_pointer].id;
        --queue_pointer;
        descer(1, queue_pointer);
        return retorno;
    }
    else
    {
        printf("%d:Queue underflow.\n", m_rank);
        exit(1);
    }
}
/* --- Priority Queue --- */

int count_set(int rank)
{
    int i;
    int counter = 0;    
    for (i = 0; i < m_qty_proccess; i++)
    {
        if (m_sets[rank][i] == 1)
        {
            counter++;
        }
    }
    return counter;
}

void init_variables()
{
    int i;
    printf("%d:Initializing variables.\n", m_rank);
    m_highest_seq = 0;
    m_expected = 0;
    m_locked = 0;
    m_has_probed = 0;
    for (i = 0; i < m_qty_proccess; i++)
        m_got_probe[i] = 0;
    for (i = 0; i < m_qty_proccess; i++)
        m_got_delayed[i] = 0;
    m_access_counter = 0;
    m_finished = 0;
}

void access_shared_resource()
{
    printf("%d:Accessing resource.\n", m_rank);
    m_access_counter++;
}

void send_request()
{
    int i;
    m_seq = m_highest_seq + 1;
    m_highest_seq = m_seq;
    m_expected = count_set(m_rank);
    for (i = 0; i < m_qty_proccess; i++)
        if (m_sets[m_rank][i] == 1)
        {
            printf("%d:Sending REQUEST to [%d].\n", m_rank, i);
            MPI_Send(&m_seq, 1, MPI_INT, i, TAG_REQUEST, MPI_COMM_WORLD);
        }
}

void make_requests()
{
    send_request();
}

void receive_request(int seq, int origin)
{
    printf("%d:Receiving REQUEST from [%d].\n", m_rank, origin);
    m_highest_seq = MAX(m_highest_seq, seq);
    if (!m_locked)
    {
        m_locked = 1;
        m_granted_node = origin;
        m_granted_seq = seq;
        m_granted_id = origin;
        printf("%d:Sending GRANTED to [%d].\n", m_rank, origin);
        MPI_Send(&seq, 1, MPI_INT, origin, TAG_GRANTED, MPI_COMM_WORLD);
    }
    else
    {
        /* add to queue */
        int aux_id = queue[1].id;
        int aux_seq = queue[1].seq;
        insercao(seq, origin);
        if ((seq < m_granted_seq || (seq == m_granted_seq && origin < m_granted_id))
                && (queue[1].id == origin)) /* && first */
        {
            if (!m_has_probed)
            {
                m_has_probed = 1;
                printf("%d:Sending PROBE to [%d].\n", m_rank, m_granted_node);
                MPI_Send(&m_granted_seq, 1, MPI_INT, m_granted_node, TAG_PROBE, MPI_COMM_WORLD);
            }
            else
            {
                printf("%d:Sending DELAYED to [%d].\n", m_rank, aux_id);
                MPI_Send(&m_granted_seq, 1, MPI_INT, aux_id, TAG_DELAYED, MPI_COMM_WORLD);
            }
        }
        else
        {
            printf("%d:Sending DELAYED to [%d].\n", m_rank, origin);
            MPI_Send(&seq, 1, MPI_INT, origin, TAG_DELAYED, MPI_COMM_WORLD);
        }
    }
}

void receive_granted(int seq, int origin)
{
    int i;
    printf("%d:Receiving GRANTED from [%d].\n", m_rank, origin);
    m_expected -= 1;
    if (m_got_delayed[origin])
        m_got_delayed[origin] = 0;
    if (m_expected == 0)
    {
        access_shared_resource();
        for (i = 0; i < m_qty_proccess; i++)
            if (m_sets[m_rank][i])
                if (m_got_probe[i])
                    m_got_probe[i] = 0;
        for (i = 0; i < m_qty_proccess; i++)
            if (m_sets[m_rank][i])
            {
                printf("%d:Sending RELEASE to [%d].\n", m_rank, i);
                MPI_Send(&m_seq, 1, MPI_INT, i, TAG_RELEASE, MPI_COMM_WORLD);
            }
        if (m_access_counter < REQUEST_ITERATIONS)
            send_request();
        else
        {
            for (i = 0; i < m_qty_proccess; i++)
                if (m_rank != i)
                {
                    int inner_msg = MSG_DEFAULT;
                    printf("%d:Sending FINISHED to [%d].\n", m_rank, i);
                    MPI_Send(&inner_msg, 1, MPI_INT, i, TAG_FINISHED, MPI_COMM_WORLD);
                }
        }
    }
}

void receive_probe(int seq, int origin)
{
    int i;
    int got_delayed = 0;
    printf("%d:Receiving PROBE from [%d].\n", m_rank, origin);
    if (seq != m_seq)
    {
        printf("%d:Receiving PROBE for old MSG from [%d].\n", m_rank, origin);
        return;
    }
    for (i = 0; i < m_qty_proccess; i++)
        if (m_sets[m_rank][i] == 1)
        {
            if (m_got_delayed[i])
            {
                got_delayed = 1;
                break;
            }
        }
    if (got_delayed)
    {
        m_expected += 1;
        printf("%d:Sending RELINQUISH to [%d].\n", m_rank, origin);
        MPI_Send(&m_seq, 1, MPI_INT, origin, TAG_RELINQUISH, MPI_COMM_WORLD);
    }
    else
    {
        m_got_probe[origin] = 1;
    }
}

void receive_delayed(int seq, int origin)
{
    int i;
    printf("%d:Receiving DELAYED from [%d].\n", m_rank, origin);
    m_got_delayed[origin] = 1;
    for (i = 0; i < m_qty_proccess; i++)
        if (m_sets[m_rank][i] == 1)
        {
            if (m_got_probe[i])
            {
                m_got_probe[i] = 0;
                m_expected += 1;
                printf("%d:Sending RELINQUISH to [%d].\n", m_rank, i);
                MPI_Send(&m_seq, 1, MPI_INT, i, TAG_RELINQUISH, MPI_COMM_WORLD);
            }
        }
}

void receive_release(int seq, int origin)
{
    printf("%d:Receiving RELEASE from [%d].\n", m_rank, origin);
    if (m_has_probed)
        m_has_probed = 0;
    if (queue_pointer == 0)
        m_locked = 0;
    else
    {
        /* let be first in queue */
        /* remove from queue */
        m_granted_node = queue[1].id;
        m_granted_seq = queue[1].seq;
        m_granted_id = queue[1].id;
        remocao();
        printf("%d:Sending GRANTED to [%d].\n", m_rank, m_granted_id);
        MPI_Send(&m_granted_seq, 1, MPI_INT, m_granted_id, TAG_GRANTED, MPI_COMM_WORLD);
    }

}

void receive_relinquish(int seq, int origin)
{
    printf("%d:Receiving RELINQUISH from [%d].\n", m_rank, origin);
    m_has_probed = 0;
    /* add to queue */
    insercao(m_granted_seq, m_granted_id);
    /* let be first in queue */
    /* remove from queue */
    m_granted_node = queue[1].id;
    m_granted_seq = queue[1].seq;
    m_granted_id = queue[1].id;
    remocao();
    printf("%d:Sending GRANTED to [%d].\n", m_rank, m_granted_id);
    MPI_Send(&m_granted_seq, 1, MPI_INT, m_granted_id, TAG_GRANTED, MPI_COMM_WORLD);
}

void receive_finished(int seq, int origin)
{
    printf("%d:Receiving FINISHED from [%d].\n", m_rank, origin);
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
    /* while (1) */
    while (m_access_counter < REQUEST_ITERATIONS || m_finished < m_qty_proccess-1)
    {
        MPI_Recv(&seq, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        origin = status.MPI_SOURCE;
        tag = status.MPI_TAG;
        // printf("%d:{Origin[%d], Tag[%d]}\n", m_rank, origin, tag);
        if (tag == TAG_REQUEST)
            receive_request(seq, origin);
        else if (tag == TAG_GRANTED)
            receive_granted(seq, origin);
        else if (tag == TAG_PROBE)
            receive_probe(seq, origin);
        else if (tag == TAG_DELAYED)
            receive_delayed(seq, origin);
        else if (tag == TAG_RELEASE)
            receive_release(seq, origin);
        else if (tag == TAG_RELINQUISH)
            receive_relinquish(seq, origin);
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
