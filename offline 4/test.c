#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: SLIGHTLY MODIFIED
 FROM VERSION 1.1 of J.F.Kurose

   This code should be used for PA2, unidirectional or bidirectional
   data transfer protocols (from A to B. Bidirectional transfer of data
   is for extra credit and is not required).  Network properties:
   - one way network delay averages five time units (longer if there
       are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
       or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
       (although some can be lost).
**********************************************************************/

#define BIDIRECTIONAL 1 /* change to 1 if you're doing extra credit */
/* and write a routine called B_output */

int dataFrame = 0;
int acknowledgment = 1;
int piggyBacking = 2;

int crc_flag;
int piggy;
char *generator;

/* a "msg" is the data unit passed from layer 5 (teachers code) to layer  */
/* 4 (students' code).  It contains the data (characters) to be delivered */
/* to layer 5 via the students transport level protocol entities.         */
struct pkt
{
    char data[4];
};

/* a packet is the data unit passed from layer 4 (students code) to layer */
/* 3 (teachers code).  Note the pre-defined packet structure, which all   */
/* students must follow. */
struct frm
{
    int type;
    int seqnum;
    int acknum;
    int checksum;
    char payload[4];
    char remainder[4];
};

/********* FUNCTION PROTOTYPES. DEFINED IN THE LATER PART******************/
void starttimer(int AorB, float increment);
void stoptimer(int AorB);
void tolayer1(int AorB, struct frm frame);
void tolayer3(int AorB, char data[4]);

struct frm frame_A;
struct frm frame_B;

struct frm ack_frame_A;
struct frm ack_frame_B;

float timerCount = 1000;

int deliver_complete_A = 1;
int deliver_complete_B = 1;

int prev_sent_seq_A = 1;
int prev_sent_seq_B = 1;

int prev_received_seq_A = 1;
int prev_received_seq_B = 1;

int outstanding_ACK_A = 0;
int outstanding_ACK_B = 0;

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/

int checkSum(struct frm frame)
{
    int sum = 0;
    int i = 0;

    sum += frame.acknum;
    sum += frame.seqnum;
    sum += frame.type;

    for (i = 0; i < 4; i++)
    {
        sum += frame.payload[i];
    }

    return sum;
}

int extract(char data, int i)
{
    return (data >> i) & 0x01;
}

char generator_polynomial[5] = "11010";

void CRC(struct frm frame, int flag)
{

    int i, j, k, l;
    int generator_polynomial_length;
    int input_bit_string_tracker = 0;
    char input_bit_string[32] = {'\0'};
    char data_transmitted[36] = {'\0'};
    char quotient[32] = {'\0'};
    char remainder[4] = {'\0'};
    char dummy_generator_polynomial[5] = {'\0'};
    char taker_from_beginning[20] = {'\0'};

    for (i = 0; i < 4; i++)
    {
        char taken = frame.payload[i];

        for (j = 7; j >= 0; j--)
        {
            input_bit_string[input_bit_string_tracker] = extract(taken, j) + '0';
            input_bit_string_tracker++;
        }
    }

    if (crc_flag == 1)
    {
        printf("\nInput Bit String : \n");
        for (i = 0; i < 32; i++)
        {
            printf("%c", input_bit_string[i]);
        }
        printf("\n");
    }

    generator_polynomial_length = strlen(generator_polynomial);
    //printf("%d\n" , generator_polynomial_length);
    //char remainder[generator_polynomial_length - 1] = {'\0'};
    //printf("%d\n" , strlen(remainder));
    strcpy(dummy_generator_polynomial, generator_polynomial);

    for (i = 0; i < generator_polynomial_length - 1; i++)
    {
        data_transmitted[32 + i] = frame.remainder[i];
    }

    for (i = 0; i < generator_polynomial_length; i++)
    {
        taker_from_beginning[i] = data_transmitted[i];
    }

    for (i = 0; i < 32; i++)
    {
        quotient[i] = taker_from_beginning[0];

        if (quotient[i] == '0')
        {
            for (j = 0; j < generator_polynomial_length; j++)
            {
                dummy_generator_polynomial[j] = '0';
            }
        }
        else
        {
            for (j = 0; j < generator_polynomial_length; j++)
            {
                dummy_generator_polynomial[j] = generator_polynomial[j];
            }
        }

        for (j = generator_polynomial_length - 1; j > 0; j--)
        {
            if (taker_from_beginning[j] == dummy_generator_polynomial[j])
            {
                remainder[j - 1] = '0';
            }
            else
            {
                remainder[j - 1] = '1';
            }
        }
        if (i != 31)
        {
            remainder[generator_polynomial_length - 1] = data_transmitted[i + generator_polynomial_length];
        }
        else
        {
            remainder[generator_polynomial_length - 1] = '\0';
        }

        strcpy(taker_from_beginning, remainder);

        //printf("%d\n" , strlen(remainder));
    }

    strcpy(remainder, taker_from_beginning);

    if (flag == 0)
    {
        strcpy(frame_A.remainder, remainder);
    }
    else if (flag == 1)
    {
        strcpy(frame_B.remainder, remainder);
    }
    else if (flag == 2)
    {
        strcpy(ack_frame_A.remainder, remainder);
    }
    else
    {
        strcpy(ack_frame_B.remainder, remainder);
    }

    if (crc_flag == 1)
    {
        printf("\nGenerator Polynomial : \n");
        for (i = 0; i < generator_polynomial_length; i++)
        {
            printf("%c", generator_polynomial[i]);
        }
        printf("\n");

        printf("\nCalculated CRC : \n");
        for (i = 0; i < 4; i++)
        {
            printf("%c", remainder[i]);
        }
        printf("\n");
    }

    // int sum = 0;
    // int i = 0;

    // sum += frame.acknum;
    // sum += frame.seqnum;
    // sum += frame.type;

    // for (i = 0; i < 4; i++)
    // {
    //     sum += frame.payload[i];
    // }

    // return sum;
}

void sendAcknowledgmentFrameA(struct frm frame)
{
    ack_frame_A.seqnum = frame.seqnum;
    ack_frame_A.acknum = frame.seqnum;
    ack_frame_A.type = acknowledgment;
    strcpy(ack_frame_A.remainder, "0000");
    //CRC(ack_frame_A, 2);

    outstanding_ACK_A = 0;
}

void sendAcknowledgmentFrameB(struct frm frame)
{
    ack_frame_B.seqnum = frame.seqnum;
    ack_frame_B.acknum = frame.seqnum;
    ack_frame_B.type = acknowledgment;
    strcpy(ack_frame_A.remainder, "0000");
    //CRC(ack_frame_B, 3);

    outstanding_ACK_B = 0;
}

/* called from layer 5, passed the data to be sent to other side */
void A_output(struct pkt packet)
{
    if (deliver_complete_A == 0)
        return;

    frame_A.seqnum = (1 + prev_sent_seq_A) % 2;

    strcpy(frame_A.payload, packet.data);

    printf("\n A receives Packet A%d from layer 3\n", frame_A.seqnum);

    if (outstanding_ACK_A == 0)
    {
        frame_A.type = dataFrame;
        frame_A.acknum = -1;
    }
    else
    {
        frame_A.type = piggyBacking;
        frame_A.acknum = prev_received_seq_A;

        outstanding_ACK_A = 0;
    }

    strcpy(frame_A.remainder, "0000");

    CRC(frame_A, 0);

    prev_sent_seq_A = (1 + prev_sent_seq_A) % 2;
    deliver_complete_A = 0;

    starttimer(0, timerCount);

    printf("\n A sends Frame A%d to B\n", frame_A.seqnum);

    printFrame(frame_A);

    tolayer1(0, frame_A);
}

/* need be completed only for extra credit */
void B_output(struct pkt packet)
{
    if (deliver_complete_B == 0)
        return;

    frame_B.seqnum = (1 + prev_sent_seq_B) % 2;

    strcpy(frame_B.payload, packet.data);

    printf(" B receives Packet B%d from layer 3\n", frame_B.seqnum);

    if (outstanding_ACK_B == 0)
    {
        frame_B.type = dataFrame;
        frame_B.acknum = -1;
    }
    else
    {
        frame_B.type = piggyBacking;
        frame_B.acknum = prev_received_seq_B;

        outstanding_ACK_B = 0;
    }

    strcpy(frame_B.remainder, "0000");

    CRC(frame_B, 1);

    prev_sent_seq_B = (1 + prev_sent_seq_B) % 2;

    deliver_complete_B = 0;

    starttimer(1, timerCount);

    printf("\n B sends Frame B%d to A\n", frame_B.seqnum);

    printFrame(frame_B);

    tolayer1(1, frame_B);
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct frm frame)
{
    int result = 0;

    // if (frame.type == acknowledgment)
    // {
    //     CRC(frame, 2);

    //     int i;
    //     for (i = 0; i < 4; i++)
    //     {
    //         if (ack_frame_A.remainder[i] != '0')
    //             result = 1;
    //     }
    // }

    // else
    // {
    //     CRC(frame, 0);

    //     int i;
    //     for (i = 0; i < 4; i++)
    //     {
    //         if (frame_A.remainder[i] != '0')
    //             result = 1;
    //     }
    // }

    int checkSumValue = checkSum(frame);

    if (checkSumValue != frame.checksum)
        result = 1;

    if (result == 1)
    {
        printf("\n corrupted frame from A side \n");
        return;
    }

    if (frame.type == dataFrame)
    {
        printf("\n A receives a data frame from layer1 \n");

        if (prev_received_seq_A == frame.seqnum)
        {
            printf("\n Duplicate frame detected in A side\n");

            sendAcknowledgmentFrameA(frame);

            printf("\n A sends acknowledgment frame to B \n");

            printFrame(ack_frame_A);

            tolayer1(0, ack_frame_A);
        }
        else
        {
            tolayer3(0, frame.payload);
            prev_received_seq_A = frame.seqnum;

            if (piggy == 1)
            {
                outstanding_ACK_A = 1;

                printf("\n A waits to send acknowledgment to B \n");
            }
            else
            {
                sendAcknowledgmentFrameA(frame);

                printf("\n A sends acknowledgment frame to B \n");

                printFrame(ack_frame_A);

                tolayer1(0, ack_frame_A);
            }
        }
    }

    else if (frame.type == piggyBacking)
    {
        printf("\n A receives a pigggybacked frame from layer 1 \n");

        if (prev_received_seq_A == frame.seqnum)
        {
            printf("\n Duplicate frame detected in A side\n");

            sendAcknowledgmentFrameA(frame);

            printf("\n A sends acknowledgment frame to B \n");

            printFrame(ack_frame_A);

            tolayer1(0, ack_frame_A);
        }

        else
        {
            stoptimer(0);

            deliver_complete_A = 1;

            printf("\n A received acknowledgment for frame A%d \n", frame.acknum);

            tolayer3(0, frame.payload);

            outstanding_ACK_A = 1;

            prev_received_seq_A = frame.seqnum;

            printf("\n A waits to send acknowledgment to B \n");
        }
    }

    else
    {
        stoptimer(0);

        deliver_complete_A = 1;

        printf("\n A receives an acknowledgment frame for frame A%d \n", frame.acknum);
    }
}

/* called when A's timer goes off */
void A_timerinterrupt(void)
{
    starttimer(0, timerCount);

    printf("\n ...........Frame resent from A side.......... \n");

    printFrame(frame_A);

    tolayer1(0, frame_A);
}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init(void)
{
    frame_A.acknum = -1;
    frame_A.seqnum = -1;
    frame_A.checksum = 0;

    ack_frame_A.acknum = -1;
    ack_frame_A.seqnum = -1;
    ack_frame_A.checksum = 0;

    int i;

    for (i = 0; i < 4; i++)
    {
        frame_A.payload[i] = '\0';
        ack_frame_A.payload[i] = '\0';
        frame_A.remainder[i] = '0';
        ack_frame_A.remainder[i] = '0';
    }
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct frm frame)
{
    int result = 0;

    // if (frame.type == acknowledgment)
    // {
    //     CRC(frame, 3);

    //     int i;
    //     for (i = 0; i < 4; i++)
    //     {
    //         if (ack_frame_B.remainder[i] != '0')
    //             result = 1;
    //     }
    // }

    // else
    // {
    //     CRC(frame, 1);

    //     int i;
    //     for (i = 0; i < 4; i++)
    //     {
    //         if (frame_B.remainder[i] != '0')
    //             result = 1;
    //     }
    // }

    int checkSumValue = checkSum(frame);

    if (checkSumValue != frame.checksum)
        result = 1;

    if (result == 1)
    {
        printf("\n Frame corrupted at B side\n");
        return;
    }

    if (frame.type == dataFrame)
    {
        printf("\n B receives a data frame from layer 1 \n");

        if (prev_received_seq_B == frame.seqnum)
        {
            printf("\n Duplicate frame detected in B \n");

            sendAcknowledgmentFrameB(frame);

            printf("\n B sends acknowledgment frame to A \n");

            printFrame(ack_frame_B);

            tolayer1(1, ack_frame_B);
        }

        else
        {
            tolayer3(1, frame.payload);
            prev_received_seq_B = frame.seqnum;

            if (piggy == 1)
            {
                outstanding_ACK_B = 1;

                printf("\n B waits to send ACK to A \n");
            }
            else
            {
                sendAcknowledgmentFrameB(frame);

                printf("\n B sends acknowledgment frame to A \n");

                printFrame(ack_frame_B);

                tolayer1(1, ack_frame_B);
            }
        }
    }

    else if (frame.type == piggyBacking)
    {
        printf("\n B receives a piggybacked frame from layer 1 \n");

        if (prev_received_seq_B == frame.seqnum)
        {
            printf("\n Duplicate frame detected in B \n");

            sendAcknowledgmentFrameB(frame);

            printf("\n B sends acknowledgment frame to A \n");

            printFrame(ack_frame_B);

            tolayer1(1, ack_frame_B);
        }

        else
        {
            stoptimer(1);

            deliver_complete_B = 1;

            printf("\n B received ACK for frame B%d \n", frame.acknum);

            tolayer3(1, frame.payload);

            outstanding_ACK_B = 1;

            prev_received_seq_B = frame.seqnum;

            printf("\n B waits to send ACK to A \n");
        }
    }

    else
    {
        stoptimer(1);

        deliver_complete_B = 1;

        printf("\n B receives an acknowledgment frame for frame B%d \n", frame.acknum);
    }
}

/* called when B's timer goes off */
void B_timerinterrupt(void)
{
    //printf("  B_timerinterrupt: B doesn't have a timer. ignore.\n");

    starttimer(1, timerCount);

    printf("\n ...........Frame resent from B side.......... \n");

    printFrame(frame_B);

    tolayer1(1, frame_B);
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init(void)
{
    frame_B.acknum = -1;
    frame_B.seqnum = -1;
    frame_B.checksum = 0;

    ack_frame_B.acknum = -1;
    ack_frame_B.seqnum = -1;
    ack_frame_B.checksum = 0;

    int i;

    for (i = 0; i < 4; i++)
    {
        frame_B.payload[i] = '\0';
        ack_frame_B.payload[i] = '\0';
        frame_B.remainder[i] = '0';
        ack_frame_B.remainder[i] = '0';
    }
}

void printFrame(struct frm frame)
{
    int i;
    printf("\nType : %d , Seq : %d , Ack : %d , Checksum : %d , Data : ", frame.type, frame.seqnum, frame.acknum, frame.checksum);
    for (i = 0; i < 4; i++)
    {
        printf("%c", frame.payload[i]);
    }
    printf("\n");

    for (i = 0; i < 4; i++)
    {
        printf("%c", frame.remainder[i]);
    }
    printf("\n");
}

/*****************************************************************
***************** NETWORK EMULATION CODE STARTS BELOW ***********
The code below emulates the layer 3 and below network environment:
    - emulates the tranmission and delivery (possibly with bit-level corruption
        and packet loss) of packets across the layer 3/4 interface
    - handles the starting/stopping of a timer, and generates timer
        interrupts (resulting in calling students timer handler).
    - generates message to be sent (passed from later 5 to 4)

THERE IS NOT REASON THAT ANY STUDENT SHOULD HAVE TO READ OR UNDERSTAND
THE CODE BELOW.  YOU SHOLD NOT TOUCH, OR REFERENCE (in your code) ANY
OF THE DATA STRUCTURES BELOW.  If you're interested in how I designed
the emulator, you're welcome to look at the code - but again, you should have
to, and you defeinitely should not have to modify
******************************************************************/

struct event
{
    float evtime;       /* event time */
    int evtype;         /* event type code */
    int eventity;       /* entity where event occurs */
    struct frm *pktptr; /* ptr to packet (if any) assoc w/ this event */
    struct event *prev;
    struct event *next;
};
struct event *evlist = NULL; /* the event list */

/* possible events: */
#define TIMER_INTERRUPT 0
#define FROM_LAYER5 1
#define FROM_LAYER3 2

#define OFF 0
#define ON 1
#define A 0
#define B 1

int TRACE = 1;   /* for my debugging */
int nsim = 0;    /* number of messages from 5 to 4 so far */
int nsimmax = 0; /* number of msgs to generate, then stop */
float time = 0.000;
float lossprob;    /* probability that a packet is dropped  */
float corruptprob; /* probability that one bit is packet is flipped */
float lambda;      /* arrival rate of messages from layer 5 */
int ntolayer1;     /* number sent into layer 3 */
int nlost;         /* number lost in media */
int ncorrupt;      /* number corrupted by media*/

void init();
void generate_next_arrival(void);
void insertevent(struct event *p);

int main()
{
    struct event *eventptr;
    struct pkt msg2give;
    struct frm pkt2give;

    int i, j;
    char c;

    init();
    A_init();
    B_init();

    while (1)
    {
        eventptr = evlist; /* get next event to simulate */
        if (eventptr == NULL)
            goto terminate;
        evlist = evlist->next; /* remove this event from event list */
        if (evlist != NULL)
            evlist->prev = NULL;
        if (TRACE >= 2)
        {
            printf("\nEVENT time: %f,", eventptr->evtime);
            printf("  type: %d", eventptr->evtype);
            if (eventptr->evtype == 0)
                printf(", timerinterrupt  ");
            else if (eventptr->evtype == 1)
                printf(", fromlayer5 ");
            else
                printf(", fromlayer3 ");
            printf(" entity: %d\n", eventptr->eventity);
        }
        time = eventptr->evtime; /* update time to next event time */
        if (eventptr->evtype == FROM_LAYER5)
        {
            if (nsim < nsimmax)
            {
                if (nsim + 1 < nsimmax)
                    generate_next_arrival(); /* set up future arrival */
                /* fill in msg to give with string of same letter */
                j = nsim % 26;
                for (i = 0; i < 4; i++)
                    msg2give.data[i] = 97 + j;
                msg2give.data[3] = 0;
                if (TRACE > 2)
                {
                    printf("          MAINLOOP: data given to student: ");
                    for (i = 0; i < 4; i++)
                        printf("%c", msg2give.data[i]);
                    printf("\n");
                }
                nsim++;
                if (eventptr->eventity == A)
                    A_output(msg2give);
                else
                    B_output(msg2give);
            }
        }
        else if (eventptr->evtype == FROM_LAYER3)
        {
            pkt2give.seqnum = eventptr->pktptr->seqnum;
            pkt2give.acknum = eventptr->pktptr->acknum;
            pkt2give.checksum = eventptr->pktptr->checksum;
            pkt2give.type = eventptr->pktptr->type;
            strcpy(pkt2give.remainder, eventptr->pktptr->remainder);
            for (i = 0; i < 4; i++)
                pkt2give.payload[i] = eventptr->pktptr->payload[i];
            if (eventptr->eventity == A) /* deliver packet by calling */
                A_input(pkt2give);       /* appropriate entity */
            else
                B_input(pkt2give);
            free(eventptr->pktptr); /* free the memory for packet */
        }
        else if (eventptr->evtype == TIMER_INTERRUPT)
        {
            if (eventptr->eventity == A)
                A_timerinterrupt();
            else
                B_timerinterrupt();
        }
        else
        {
            printf("INTERNAL PANIC: unknown event type \n");
        }
        free(eventptr);
    }

terminate:
    printf(
        " Simulator terminated at time %f\n after sending %d msgs from layer5\n",
        time, nsim);
}

void init() /* initialize the simulator */
{
    int i;
    float sum, avg;
    float jimsrand();

    printf("-----  Stop and Wait Network Simulator Version 1.1 -------- \n\n");

    printf("Enter CRC Steps:");
    scanf("%d", &crc_flag);
    printf("Enter piggybacking:");
    scanf("%d", &piggy);
    //printf("Enter Generator Polynomial:");
    //scanf("%s", generator_polynomial);
    printf("Enter the number of messages to simulate: ");
    scanf("%d", &nsimmax);
    printf("Enter  packet loss probability [enter 0.0 for no loss]:");
    scanf("%f", &lossprob);
    printf("Enter packet corruption probability [0.0 for no corruption]:");
    scanf("%f", &corruptprob);
    printf("Enter average time between messages from sender's layer5 [ > 0.0]:");
    scanf("%f", &lambda);
    printf("Enter TRACE:");
    scanf("%d", &TRACE);

    srand(9999); /* init random number generator */
    sum = 0.0;   /* test random number generator for students */
    for (i = 0; i < 1000; i++)
        sum = sum + jimsrand(); /* jimsrand() should be uniform in [0,1] */
    avg = sum / 1000.0;
    if (avg < 0.25 || avg > 0.75)
    {
        printf("It is likely that random number generation on your machine\n");
        printf("is different from what this emulator expects.  Please take\n");
        printf("a look at the routine jimsrand() in the emulator code. Sorry. \n");
        exit(1);
    }

    ntolayer1 = 0;
    nlost = 0;
    ncorrupt = 0;

    time = 0.0;              /* initialize time to 0.0 */
    generate_next_arrival(); /* initialize event list */
}

/****************************************************************************/
/* jimsrand(): return a float in range [0,1].  The routine below is used to */
/* isolate all random number generation in one location.  We assume that the*/
/* system-supplied rand() function return an int in therange [0,mmm]        */
/****************************************************************************/
float jimsrand(void)
{
    double mmm = RAND_MAX;
    float x;          /* individual students may need to change mmm */
    x = rand() / mmm; /* x should be uniform in [0,1] */
    return (x);
}

/********************* EVENT HANDLINE ROUTINES *******/
/*  The next set of routines handle the event list   */
/*****************************************************/

void generate_next_arrival(void)
{
    double x, log(), ceil();
    struct event *evptr;
    float ttime;
    int tempint;

    if (TRACE > 2)
        printf("          GENERATE NEXT ARRIVAL: creating new arrival\n");

    x = lambda * jimsrand() * 2; /* x is uniform on [0,2*lambda] */
    /* having mean of lambda        */
    evptr = (struct event *)malloc(sizeof(struct event));
    evptr->evtime = time + x;
    evptr->evtype = FROM_LAYER5;
    if (BIDIRECTIONAL && (jimsrand() > 0.5))
        evptr->eventity = B;
    else
        evptr->eventity = A;
    insertevent(evptr);
}

void insertevent(struct event *p)
{
    struct event *q, *qold;

    if (TRACE > 2)
    {
        printf("            INSERTEVENT: time is %lf\n", time);
        printf("            INSERTEVENT: future time will be %lf\n", p->evtime);
    }
    q = evlist; /* q points to header of list in which p struct inserted */

    if (q == NULL) /* list is empty */
    {
        evlist = p;
        p->next = NULL;
        p->prev = NULL;
    }
    else
    {
        for (qold = q; q != NULL && p->evtime > q->evtime; q = q->next)
            qold = q;
        if (q == NULL) /* end of list */
        {
            qold->next = p;
            p->prev = qold;
            p->next = NULL;
        }
        else if (q == evlist) /* front of list */
        {
            p->next = evlist;
            p->prev = NULL;
            p->next->prev = p;
            evlist = p;
        }
        else /* middle of list */
        {
            p->next = q;
            p->prev = q->prev;
            q->prev->next = p;
            q->prev = p;
        }
    }
}

void printevlist(void)
{
    struct event *q;
    int i;
    printf("--------------\nEvent List Follows:\n");
    for (q = evlist; q != NULL; q = q->next)
    {
        printf("Event time: %f, type: %d entity: %d\n", q->evtime, q->evtype,
               q->eventity);
    }
    printf("--------------\n");
}

/********************** Student-callable ROUTINES ***********************/

/* called by students routine to cancel a previously-started timer */
void stoptimer(int AorB /* A or B is trying to stop timer */)
{
    struct event *q, *qold;

    if (TRACE > 2)
        printf("          STOP TIMER: stopping timer at %f\n", time);
    /* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next)  */
    for (q = evlist; q != NULL; q = q->next)
        if ((q->evtype == TIMER_INTERRUPT && q->eventity == AorB))
        {
            /* remove this event */
            if (q->next == NULL && q->prev == NULL)
                evlist = NULL;        /* remove first and only event on list */
            else if (q->next == NULL) /* end of list - there is one in front */
                q->prev->next = NULL;
            else if (q == evlist) /* front of list - there must be event after */
            {
                q->next->prev = NULL;
                evlist = q->next;
            }
            else /* middle of list */
            {
                q->next->prev = q->prev;
                q->prev->next = q->next;
            }
            free(q);
            return;
        }
    printf("Warning: unable to cancel your timer. It wasn't running.\n");
}

void starttimer(int AorB /* A or B is trying to start timer */, float increment)
{
    struct event *q;
    struct event *evptr;

    if (TRACE > 2)
        printf("          START TIMER: starting timer at %f\n", time);
    /* be nice: check to see if timer is already started, if so, then  warn */
    /* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next)  */
    for (q = evlist; q != NULL; q = q->next)
        if ((q->evtype == TIMER_INTERRUPT && q->eventity == AorB))
        {
            printf("Warning: attempt to start a timer that is already started\n");
            return;
        }

    /* create future event for when timer goes off */
    evptr = (struct event *)malloc(sizeof(struct event));
    evptr->evtime = time + increment;
    evptr->evtype = TIMER_INTERRUPT;
    evptr->eventity = AorB;
    insertevent(evptr);
}

/************************** TOLAYER3 ***************/
void tolayer1(int AorB, struct frm frame)
{
    struct frm *mypktptr;
    struct event *evptr, *q;
    float lastime, x;
    int i;

    ntolayer1++;

    /* simulate losses: */
    if (jimsrand() < lossprob)
    {
        nlost++;
        if (TRACE > 0)
            printf("          TOLAYER1: packet being lost\n");
        return;
    }

    /* make a copy of the packet student just gave me since he/she may decide */
    /* to do something with the packet after we return back to him/her */
    mypktptr = (struct frm *)malloc(sizeof(struct frm));
    mypktptr->seqnum = frame.seqnum;
    mypktptr->acknum = frame.acknum;
    mypktptr->checksum = frame.checksum;
    mypktptr->type = frame.type;
    strcpy(mypktptr->remainder, frame.remainder);
    for (i = 0; i < 4; i++)
        mypktptr->payload[i] = frame.payload[i];
    if (TRACE > 2)
    {
        printf("          TOLAYER1: seq: %d, ack %d, check: %d ", mypktptr->seqnum,
               mypktptr->acknum, mypktptr->checksum);
        for (i = 0; i < 4; i++)
            printf("%c", mypktptr->payload[i]);
        printf("\n");
    }

    /* create future event for arrival of packet at the other side */
    evptr = (struct event *)malloc(sizeof(struct event));
    evptr->evtype = FROM_LAYER3;      /* packet will pop out from layer3 */
    evptr->eventity = (AorB + 1) % 2; /* event occurs at other entity */
    evptr->pktptr = mypktptr;         /* save ptr to my copy of packet */
    /* finally, compute the arrival time of packet at the other end.
       medium can not reorder, so make sure packet arrives between 1 and 10
       time units after the latest arrival time of packets
       currently in the medium on their way to the destination */
    lastime = time;
    /* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next) */
    for (q = evlist; q != NULL; q = q->next)
        if ((q->evtype == FROM_LAYER3 && q->eventity == evptr->eventity))
            lastime = q->evtime;
    evptr->evtime = lastime + 1 + 9 * jimsrand();

    /* simulate corruption: */
    if (jimsrand() < corruptprob)
    {
        ncorrupt++;
        if ((x = jimsrand()) < .75)
            mypktptr->payload[0] = 'Z'; /* corrupt payload */
        else if (x < .875)
            mypktptr->seqnum = 999999;
        else
            mypktptr->acknum = 999999;
        if (TRACE > 0)
            printf("          TOLAYER1: packet being corrupted\n");
    }

    if (TRACE > 2)
        printf("          TOLAYER1: scheduling arrival on other side\n");
    insertevent(evptr);
}

void tolayer3(int AorB, char data[4])
{
    int i;
    if (TRACE > 2)
    {
        printf("          TOLAYER3: data received: ");
        for (i = 0; i < 4; i++)
            printf("%c", data[i]);
        printf("\n");
    }
}
