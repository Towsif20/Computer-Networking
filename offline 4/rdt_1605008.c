#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

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

/* a "pkt" is the data unit passed from layer 5 (teachers code) to layer  */
/* 4 (students' code).  It contains the data (characters) to be delivered */
/* to layer 5 via the students transport level protocol entities.         */
struct pkt
{
    char data[4];
};

/* a packet is the data unit passed from layer 4 (students code) to layer */
/* 3 (teachers code).  Note the pre-defined packet structure, which all   */
/* students must follow. */
#define data_frame 0
#define acknowledgment 1
#define piggy 2

struct frm
{
    int type;
    int seqnum;
    int acknum;
    int checksum;
    char payload[4];
};

/********* FUNCTION PROTOTYPES. DEFINED IN THE LATER PART******************/
void starttimer(int AorB, float increment);
void stoptimer(int AorB);
void tolayer1(int AorB, struct frm packet);
void tolayer3(int AorB, char datasent[4]);

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/
int extract(char data, int i)
{
    return (data >> i) & 0x01;
}

int extract2(int data, int i)
{
    return (data >> i) & 0x01;
}

int crc_steps;
int piggybacking;
char generator_polynomial[20] = {'\0'};
//char generator_polynomial[5] = "11001";
//generator_polynomial = {'1' , '1' , '0' , '0' , '1'};

int checksumming(struct frm frame)
{
    int i,j,k,l;
    int generator_polynomial_length;
    int input_bit_string_tracker = 0;
    char input_bit_string[128] = {'\0'};
    char data_transmitted[150] = {'\0'};
    char quotient[128] = {'\0'};
    char remainder[20] = {'\0'};
    char dummy_generator_polynomial[20] = {'\0'};
    char taker_from_beginning[20] = {'\0'};

    for(i=0 ; i<4 ; i++)
    {
        char taken = frame.payload[i];

        for(j=7 ; j>=0 ; j--)
        {
            input_bit_string[input_bit_string_tracker] = extract(taken , j) + '0';
            input_bit_string_tracker++;
        }
    }

    int seqnum = frame.seqnum;
    for(k=31 ; k>=0 ; k--)
    {
        input_bit_string[input_bit_string_tracker] = extract2(seqnum , k) + '0';
        input_bit_string_tracker++;
    }

    int acknum = frame.acknum;
    for(k=31 ; k>=0 ; k--)
    {
        input_bit_string[input_bit_string_tracker] = extract2(acknum , k) + '0';
        input_bit_string_tracker++;
    }

    int type = frame.type;
    for(k=31 ; k>=0 ; k--)
    {
        input_bit_string[input_bit_string_tracker] = extract2(type , k) + '0';
        input_bit_string_tracker++;
    }

    if(crc_steps == 1)
    {
        printf("\nInput Bit String : \n");
        for(i=0 ; i<128 ; i++)
        {
            printf("%c" , input_bit_string[i]);
        }
        printf("\n");
    }

    for(l=0 ; l<128 ; l++)
    {
        data_transmitted[l] = input_bit_string[l];
    }

    generator_polynomial_length = strlen(generator_polynomial);
    //printf("%d\n" , generator_polynomial_length);
    //char remainder[generator_polynomial_length - 1] = {'\0'};
    //printf("%d\n" , strlen(remainder));
    strcpy(dummy_generator_polynomial , generator_polynomial);

    for(i=0 ; i<generator_polynomial_length-1 ; i++)
    {
        data_transmitted[128+i] = '0';
    }

    for(i=0 ; i<generator_polynomial_length ; i++)
    {
        taker_from_beginning[i] = data_transmitted[i];
    }

    for(i=0 ; i<128 ; i++)
    {
        quotient[i] = taker_from_beginning[0];

        if(quotient[i] == '0')
        {
            for(j=0 ; j<generator_polynomial_length ; j++)
            {
                dummy_generator_polynomial[j] = '0';
            }
        }
        else
        {
            for(j=0 ; j<generator_polynomial_length ; j++)
            {
                dummy_generator_polynomial[j] = generator_polynomial[j];
            }
        }

        for(j=generator_polynomial_length-1 ; j>0 ; j--)
        {
            if(taker_from_beginning[j] == dummy_generator_polynomial[j])
            {
                remainder[j-1] = '0';
            }
            else
            {
                remainder[j-1] = '1';
            }
        }
        if(i != 127){
            remainder[generator_polynomial_length-1] = data_transmitted[i+generator_polynomial_length];
        }
        else
        {
            remainder[generator_polynomial_length-1] = '\0';
        }

        strcpy(taker_from_beginning , remainder);

        //printf("%d\n" , strlen(remainder));
    }

    //strcpy(remainder , taker_from_beginning);

    int remainder_size = strlen(remainder);
    int crc[remainder_size];
    int crc_converted_to_integer = 0;
    int base = 1;

    if(crc_steps == 1)
    {
        printf("\nGenerator Polynomial : \n");
        for(i=0 ; i<generator_polynomial_length ; i++)
        {
            printf("%c" , generator_polynomial[i]);
        }
        printf("\n");

        printf("\nCalculated CRC : \n");
        for(i=0 ; i<remainder_size ; i++)
        {
            printf("%c" , remainder[i]);
        }
        printf("\n");
    }
    //printf("\n%d\n" , remainder_size);

    for(i=0 ; i<remainder_size ; i++)
    {
        crc[i] = remainder[i] - '0';
    }

    for(j=remainder_size-1 ; j>=0 ; j--)
    {
        int last_digit = crc[j];

        crc_converted_to_integer += last_digit * base;

        base = base * 2;
    }

    return crc_converted_to_integer;

    /*int sum = 0;
    int i = 0;

    sum += frame.acknum;
    sum += frame.seqnum;
    sum += frame.type;

    for(i=0 ; i<4 ; i++)
    {
        sum += frame.payload[i];
    }

    return sum;*/
}

void frame_printer(struct frm frame)
{
    int i;
    printf("Type : %d , Seq : %d , Ack : %d , Checksum : %d , Data : " , frame.type , frame.seqnum , frame.acknum , frame.checksum);
    for(i=0 ; i<4 ; i++)
    {
        printf("%c" , frame.payload[i]);
    }
    printf("\n\n");
}

struct frm frame_of_A;
struct frm frame_of_B;

struct frm acknowledgment_frame_of_A;
struct frm acknowledgment_frame_of_B;

#define true 1
#define false 0
int frame_of_A_deliver_complete = true;
int frame_of_B_deliver_complete = true;

int previous_sent_frame_seqnum_of_A = 1;
int previous_sent_frame_seqnum_of_B = 1;
int previous_received_frame_seqnum_of_A = 1;
int previous_received_frame_seqnum_of_B = 1;

int expecting_seqnum_of_A = 0;

float interrupt_time = 200.0;

int successfully_sent = 0;
int sent = 0;

int outstanding_ACK_B = 0;
int outstanding_ACK_A = 0;

//char generator_polynomial[20];

/* called from layer 5, passed the data to be sent to other side */
void A_output(struct pkt packet)
{
    if(frame_of_A_deliver_complete == true)
    {
        frame_of_A.seqnum = 1 - previous_sent_frame_seqnum_of_A;

        strcpy(frame_of_A.payload , packet.data);

        printf("\n----------------------A receives Packet_A%d from layer3-----------------------\n" , frame_of_A.seqnum);

        if(outstanding_ACK_A == 1)
        {
            frame_of_A.type = piggy;
            frame_of_A.acknum = previous_received_frame_seqnum_of_A;

            outstanding_ACK_A = 0;
        }
        else
        {
            frame_of_A.type = data_frame;
            frame_of_A.acknum = -1;
        }

        frame_of_A.checksum = checksumming(frame_of_A);

        previous_sent_frame_seqnum_of_A = 1 - previous_sent_frame_seqnum_of_A;
        frame_of_A_deliver_complete = false;

        starttimer(0 , interrupt_time);

        printf("\n----------------------A sends Frame_A%d to B -------------------------\n" , frame_of_A.seqnum);

        frame_printer(frame_of_A);

        tolayer1(0 , frame_of_A);
        /*frame_of_A.seqnum = 1 - previous_sent_frame_seqnum_of_A;
        frame_of_A.acknum = -1;
        frame_of_A.type = data_frame;

        strcpy(frame_of_A.payload , message.data);

        frame_of_A.checksum = checksumming(frame_of_A);

        previous_sent_frame_seqnum_of_A = 1 - previous_sent_frame_seqnum_of_A;
        frame_of_A_deliver_complete = false;

        starttimer(0 , interrupt_time);

        //tolayer1(0 , frame_of_A);

        sent++;
        printf("\n----------------------Packet sent from A side -------------------------\n");
        printf("\n-------------------------Sent count : %d --------------------\n" , sent);

        tolayer1(0 , frame_of_A);*/
    }
}

/* need be completed only for extra credit */
void B_output(struct pkt packet)
{
    if(frame_of_B_deliver_complete == true)
    {
        frame_of_B.seqnum = 1 - previous_sent_frame_seqnum_of_B;

        strcpy(frame_of_B.payload , packet.data);

        printf("\n----------------------B receives Packet_B%d from layer3-----------------------\n" , frame_of_B.seqnum);

        if(outstanding_ACK_B == 1)
        {
            frame_of_B.type = piggy;
            frame_of_B.acknum = previous_received_frame_seqnum_of_B;

            outstanding_ACK_B = 0;
        }
        else
        {
            frame_of_B.type = data_frame;
            frame_of_B.acknum = -1;
        }

        frame_of_B.checksum = checksumming(frame_of_B);

        previous_sent_frame_seqnum_of_B = 1 - previous_sent_frame_seqnum_of_B;
        frame_of_B_deliver_complete = false;

        starttimer(1 , interrupt_time);

        printf("\n----------------------B sends Frame_B%d to A -------------------------\n" , frame_of_B.seqnum);

        frame_printer(frame_of_B);

        tolayer1(1 , frame_of_B);
    }
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct frm frame)
{
    int received_frames_checksum = checksumming(frame);
    printf("%d , %d" , received_frames_checksum , frame.checksum);

    if(received_frames_checksum == frame.checksum)
    {
        //printf("\n----------------------A receives a data Frame from layer1----------------------------\n");

        if(frame.type == data_frame)
        {
            printf("\n----------------------A receives a data Frame from layer1----------------------------\n");

            if(previous_received_frame_seqnum_of_A == frame.seqnum)
            {
                acknowledgment_frame_of_A.seqnum = frame.seqnum;
                acknowledgment_frame_of_A.acknum = frame.seqnum;
                acknowledgment_frame_of_A.type = acknowledgment;
                acknowledgment_frame_of_A.checksum = checksumming(acknowledgment_frame_of_A);

                outstanding_ACK_A = 0;

                printf("\n-----------------------Duplicate Frame has been received--------------------------\n");
                printf("-----------------------A sends ACK Frame to B ------------------------\n");
                //printf("%d" , acknowledgment_frame_of_A.checksum);

                frame_printer(acknowledgment_frame_of_A);

                tolayer1(0 , acknowledgment_frame_of_A);
            }
            else
            {
                tolayer3(0 , frame.payload);

                if(piggybacking == 1)
                {
                    outstanding_ACK_A = 1;

                    previous_received_frame_seqnum_of_A = frame.seqnum;

                    printf("\n------------------------A waits to send ACK to B--------------------------\n");
                }
                else
                {
                    acknowledgment_frame_of_A.seqnum = frame.seqnum;
                    acknowledgment_frame_of_A.acknum = frame.seqnum;
                    acknowledgment_frame_of_A.type = acknowledgment;
                    acknowledgment_frame_of_A.checksum = checksumming(acknowledgment_frame_of_A);

                    printf("-----------------------A sends ACK Frame to B ------------------------\n");

                    previous_received_frame_seqnum_of_A = frame.seqnum;

                    frame_printer(acknowledgment_frame_of_A);

                    tolayer1(0 , acknowledgment_frame_of_A);
                }
            }

        }
        else if(frame.type == acknowledgment)
        {
            stoptimer(0);

            frame_of_A_deliver_complete = true;

            printf("\n------------------------A receives a ACK Frame from layer1--------------------------\n");
            printf("\n------------------------A received ACK for Frame_A%d-------------------------\n" , frame.acknum);
        }
        else
        {
            //stoptimer(1);

            //frame_of_B_deliver_complete = true;

            printf("\n------------------------A receives a data-ACK Frame from layer1--------------------------\n");

            if(previous_received_frame_seqnum_of_A == frame.seqnum)
            {
                acknowledgment_frame_of_A.seqnum = frame.seqnum;
                acknowledgment_frame_of_A.acknum = frame.seqnum;
                acknowledgment_frame_of_A.type = acknowledgment;
                acknowledgment_frame_of_A.checksum = checksumming(acknowledgment_frame_of_A);

                outstanding_ACK_A = 0;

                printf("\n-----------------------Duplicate Frame has been received--------------------------\n");
                printf("-----------------------A sends ACK Frame to B ------------------------\n");

                frame_printer(acknowledgment_frame_of_A);

                tolayer1(0 , acknowledgment_frame_of_A);
            }
            else
            {
                stoptimer(0);

                frame_of_A_deliver_complete = true;

                printf("\n------------------------A received ACK for Frame_A%d-------------------------\n" , frame.acknum);

                tolayer3(0 , frame.payload);

                outstanding_ACK_A = 1;

                previous_received_frame_seqnum_of_A = frame.seqnum;

                printf("\n------------------------A waits to send ACK to B--------------------------\n");
            }
        }
        /*stoptimer(0);

        tolayer3(0 , packet.payload);

        expecting_seqnum_of_A = 1 - expecting_seqnum_of_A;
        frame_of_A_deliver_complete = true;

        successfully_sent++;
        printf("\n------------------------Acknowledgment received at A side ------------------------\n");
        printf("\n ------------------------Success Count : %d -----------------------\n" , successfully_sent);*/
    }
    else
    {
        printf("\n--------------------Corrupted Frame at A side------------------------\n");
    }
}

/* called when A's timer goes off */
void A_timerinterrupt(void)
{
    starttimer(0 , interrupt_time);

    printf("\n++++++++++++++++++ A re-sends Frame_A%d to B +++++++++++++++++++\n" , frame_of_A.seqnum);

    frame_printer(frame_of_A);

    tolayer1(0 , frame_of_A);
    //printf("\n++++++++++++++++++ Re-sent the Packet +++++++++++++++++++\n");
}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init(void)
{
    frame_of_A.acknum = -1;
    frame_of_A.seqnum = -1;
    frame_of_A.checksum = 0;

    acknowledgment_frame_of_A.acknum = -1;
    acknowledgment_frame_of_A.seqnum = -1;
    acknowledgment_frame_of_A.checksum = 0;

    char nothing = '\0';
    int i;

    for(i=0 ; i<4 ; i++)
    {
        frame_of_A.payload[i] = nothing;
        acknowledgment_frame_of_A.payload[i] = nothing;
    }
}

/* Note that with simplex transfer from a-to-B, there is no B_output() */

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct frm frame)
{
    int received_frames_checksum = checksumming(frame);
    printf("%d , %d" , received_frames_checksum , frame.checksum);

    if(received_frames_checksum == frame.checksum)
    {
        //printf("\n----------------------B receives a data Frame from layer1----------------------------\n");

        if(frame.type == data_frame)
        {
            printf("\n----------------------B receives a data Frame from layer1----------------------------\n");

            if(previous_received_frame_seqnum_of_B == frame.seqnum)
            {
                acknowledgment_frame_of_B.seqnum = frame.seqnum;
                acknowledgment_frame_of_B.acknum = frame.seqnum;
                acknowledgment_frame_of_B.type = acknowledgment;
                acknowledgment_frame_of_B.checksum = checksumming(acknowledgment_frame_of_B);

                outstanding_ACK_B = 0;

                printf("\n-----------------------Duplicate Frame has been received--------------------------\n");
                printf("-----------------------B sends ACK Frame to A ------------------------\n");

                frame_printer(acknowledgment_frame_of_B);

                tolayer1(1 , acknowledgment_frame_of_B);
            }
            else
            {
                tolayer3(1 , frame.payload);

                if(piggybacking == 1)
                {
                    outstanding_ACK_B = 1;

                    previous_received_frame_seqnum_of_B = frame.seqnum;

                    printf("\n------------------------B waits to send ACK to A--------------------------\n");
                }
                else
                {
                    acknowledgment_frame_of_B.seqnum = frame.seqnum;
                    acknowledgment_frame_of_B.acknum = frame.seqnum;
                    acknowledgment_frame_of_B.type = acknowledgment;
                    acknowledgment_frame_of_B.checksum = checksumming(acknowledgment_frame_of_B);

                    printf("-----------------------B sends ACK Frame to A ------------------------\n");

                    previous_received_frame_seqnum_of_B = frame.seqnum;

                    frame_printer(acknowledgment_frame_of_B);

                    tolayer1(1 , acknowledgment_frame_of_B);
                }
            }

        }
        else if(frame.type == acknowledgment)
        {
            stoptimer(1);

            frame_of_B_deliver_complete = true;

            printf("\n------------------------B receives a ACK Frame from layer1--------------------------\n");
            printf("\n------------------------B received ACK for Frame_B%d-------------------------\n" , frame.acknum);
        }
        else
        {
            //stoptimer(1);

            //frame_of_B_deliver_complete = true;

            printf("\n------------------------B receives a data-ACK Frame from layer1--------------------------\n");

            if(previous_received_frame_seqnum_of_B == frame.seqnum)
            {
                acknowledgment_frame_of_B.seqnum = frame.seqnum;
                acknowledgment_frame_of_B.acknum = frame.seqnum;
                acknowledgment_frame_of_B.type = acknowledgment;
                acknowledgment_frame_of_B.checksum = checksumming(acknowledgment_frame_of_B);

                outstanding_ACK_B = 0;

                printf("\n-----------------------Duplicate Frame has been received--------------------------\n");
                printf("-----------------------B sends ACK Frame to A ------------------------\n");

                frame_printer(acknowledgment_frame_of_B);

                tolayer1(1 , acknowledgment_frame_of_B);
            }
            else
            {
                stoptimer(1);

                frame_of_B_deliver_complete = true;

                printf("\n------------------------B received ACK for Frame_B%d-------------------------\n" , frame.acknum);

                tolayer3(1 , frame.payload);

                outstanding_ACK_B = 1;

                previous_received_frame_seqnum_of_B = frame.seqnum;

                printf("\n------------------------B waits to send ACK to A--------------------------\n");
            }
        }

        /*if(previous_received_frame_seqnum_of_B != packet.seqnum)
        {
            tolayer3(1 , packet.payload);

            previous_received_frame_seqnum_of_B = packet.seqnum;

            printf("\n------------------ Packet received at B side -------------------\n");
        }

        frame_of_B.seqnum = packet.seqnum;
        frame_of_B.acknum = packet.seqnum;
        frame_of_B.type = acknowledgment;

        if(packet.seqnum == 0)
        {
            strcpy(frame_of_B.payload , "00000");
        }
        else
        {
            strcpy(frame_of_B.payload , "11111");
        }

        frame_of_B.checksum = checksumming(frame_of_B);

        printf("\n-----------------------Acknowledgment sent from B side -------------------------\n");

        tolayer1(1 , frame_of_B);*/
    }
    else
    {
        printf("\n-----------------------Corrupted Frame at B side-----------------------------\n");
    }
}

/* called when B's timer goes off */
void B_timerinterrupt(void)
{
    starttimer(1 , interrupt_time);

    printf("\n++++++++++++++++++ B re-sends Frame_B%d to A +++++++++++++++++++\n" , frame_of_B.seqnum);
    //printf("%d" , frame_of_B.checksum);

    frame_printer(frame_of_B);

    tolayer1(1 , frame_of_B);
    //printf("  B_timerinterrupt: B doesn't have a timer. ignore.\n");
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init(void)
{
    acknowledgment_frame_of_B.acknum = -1;
    acknowledgment_frame_of_B.seqnum = -1;
    acknowledgment_frame_of_B.checksum = 0;

    frame_of_B.acknum = -1;
    frame_of_B.seqnum = -1;
    frame_of_B.checksum = 0;

    char nothing = '\0';
    int i;

    for(i=0 ; i<4 ; i++)
    {
        frame_of_B.payload[i] = nothing;
        acknowledgment_frame_of_B.payload[i] = nothing;
    }

    //strcpy(frame_of_B.payload , "11111");

    //frame_of_B.checksum = checksumming(frame_of_B);
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
#define FROM_LAYER3 1
#define FROM_LAYER1 2

#define OFF 0
#define ON 1
#define A 0
#define B 1

int TRACE = 1;     /* for my debugging */
int nsim = 0;      /* number of messages from 5 to 4 so far */
int nsimmax = 0;   /* number of msgs to generate, then stop */
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
                printf(", fromlayer3 ");
            else
                printf(", fromlayer1 ");
            printf(" entity: %d\n", eventptr->eventity);
        }
        time = eventptr->evtime; /* update time to next event time */
        if (eventptr->evtype == FROM_LAYER3)
        {
            if (nsim < nsimmax)
            {
                if (nsim + 1 < nsimmax)
                    generate_next_arrival(); /* set up future arrival */
                /* fill in pkt to give with string of same letter */
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
        else if (eventptr->evtype == FROM_LAYER1)
        {
            pkt2give.type = eventptr->pktptr->type;
            pkt2give.seqnum = eventptr->pktptr->seqnum;
            pkt2give.acknum = eventptr->pktptr->acknum;
            pkt2give.checksum = eventptr->pktptr->checksum;
            for (i = 0; i < 4; i++)
                pkt2give.payload[i] = eventptr->pktptr->payload[i];
            if (eventptr->eventity == A) /* deliver packet by calling */
                A_input(pkt2give); /* appropriate entity */
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
    printf("Enter the number of messages to simulate: ");
    scanf("%d",&nsimmax);
    printf("Enter  packet loss probability [enter 0.0 for no loss]:");
    scanf("%f",&lossprob);
    printf("Enter packet corruption probability [0.0 for no corruption]:");
    scanf("%f",&corruptprob);
    printf("Enter average time between messages from sender's layer5 [ > 0.0]:");
    scanf("%f",&lambda);
    printf("Enter TRACE:");
    scanf("%d",&TRACE);
    //clrscr();
    printf("Enter CRC Steps:");
    scanf("%d" , &crc_steps);
    printf("Enter piggybacking:");
    scanf("%d" , &piggybacking);
    printf("Enter Generator Polynomial:");
    scanf("%s" , generator_polynomial);
    //gets(generator_polynomial);

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
    float x;                 /* individual students may need to change mmm */
    x = rand() / mmm;        /* x should be uniform in [0,1] */
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
    evptr->evtype = FROM_LAYER3;
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
    q = evlist;      /* q points to header of list in which p struct inserted */
    if (q == NULL)   /* list is empty */
    {
        evlist = p;
        p->next = NULL;
        p->prev = NULL;
    }
    else
    {
        for (qold = q; q != NULL && p->evtime > q->evtime; q = q->next)
            qold = q;
        if (q == NULL)   /* end of list */
        {
            qold->next = p;
            p->prev = qold;
            p->next = NULL;
        }
        else if (q == evlist)     /* front of list */
        {
            p->next = evlist;
            p->prev = NULL;
            p->next->prev = p;
            evlist = p;
        }
        else     /* middle of list */
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
                evlist = NULL;          /* remove first and only event on list */
            else if (q->next == NULL) /* end of list - there is one in front */
                q->prev->next = NULL;
            else if (q == evlist)   /* front of list - there must be event after */
            {
                q->next->prev = NULL;
                evlist = q->next;
            }
            else     /* middle of list */
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
void tolayer1(int AorB, struct frm packet)
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
    mypktptr->type = packet.type;
    mypktptr->seqnum = packet.seqnum;
    mypktptr->acknum = packet.acknum;
    mypktptr->checksum = packet.checksum;
    for (i = 0; i < 4; i++)
        mypktptr->payload[i] = packet.payload[i];
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
    evptr->evtype = FROM_LAYER1;      /* packet will pop out from layer3 */
    evptr->eventity = (AorB + 1) % 2; /* event occurs at other entity */
    evptr->pktptr = mypktptr;         /* save ptr to my copy of packet */
    /* finally, compute the arrival time of packet at the other end.
       medium can not reorder, so make sure packet arrives between 1 and 10
       time units after the latest arrival time of packets
       currently in the medium on their way to the destination */
    lastime = time;
    /* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next) */
    for (q = evlist; q != NULL; q = q->next)
        if ((q->evtype == FROM_LAYER1 && q->eventity == evptr->eventity))
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

void tolayer3(int AorB, char datasent[4])
{
    int i;
    if (TRACE > 2)
    {
        printf("          TOLAYER3: data received: ");
        for (i = 0; i < 4; i++)
            printf("%c", datasent[i]);
        printf("\n");
    }
}
