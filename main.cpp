/***** DID NOT HANDLE MOVES FROM FOUNDATION BACK TO A SLOT ************/

#include <stdio.h>
#include <malloc.h>
#include <errno.h>

typedef enum { MOVE_FOUND=0, MOVE_SLOT, MOVE_HOLE } MOVE_TYPE;
typedef enum { CLUB=0, DIAMOND, HEART, SPADE } SUIT;
typedef enum { ACE=-1, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN, JACK, QUEEN, KING, MAX_RANK } RANK;

typedef struct {
    SUIT mysuit;
    RANK myrank;
} CARD;

#define MAXSLOTCOUNT 17
typedef struct {
    CARD mycards[MAXSLOTCOUNT]; //maximum card count in a slot is 17 because X X X X X K Q J 10 9 8 7 6 5 4 3 2
    int  count;
} SLOT;

#define GOTO_FOUNDATION 99
#define MAX_MOVE   4


typedef struct {
    SLOT   slot[8];
    RANK   foundation[4]; //4 suits

    bool   ordered_slot[8];

    int    slotMoveCount[MAX_RANK];  //total number of possible slot moves by rank, max 4
    int    holeMoveCount[MAX_RANK];  //total number of hole moves by rank, max 4
    int    foundMoveCount[MAX_RANK]; //total number of foundation moves by rank, max 4

    int    slotMoveSrc[MAX_RANK][MAX_MOVE];    //slot 0 - 7
    int    slotMoveDest[MAX_RANK][MAX_MOVE];   //slot 0 - 7
    SUIT   slotMoveSuit[MAX_RANK][MAX_MOVE];   

    int    holeMoveSrc[MAX_RANK][MAX_MOVE];    //slot 0 - 7
    int    holeMoveDest[MAX_RANK][MAX_MOVE];   //slot 0 - 7
    SUIT   holeMoveSuit[MAX_RANK][MAX_MOVE];   

    int    foundMoveSrc[MAX_RANK][MAX_MOVE];   //slot 0 - 7
    SUIT   foundMoveSuit[MAX_RANK][MAX_MOVE];

    void   *slotMoveChild[MAX_RANK][MAX_MOVE];
    void   *holeMoveChild[MAX_RANK][MAX_MOVE];
    void   *foundMoveChild[MAX_RANK][MAX_MOVE];

    void   *parent; //root has NULL parent, all children have parent
    int    level;

    MOVE_TYPE parent_move_type;
    CARD      parent_move_card;
    int       parent_move_index;

    /***************************** DO NOT IMPLEMENT YET *******************************************/
    //int  foundMoveCount[4]; //total number of possible moves from last card in a foundation
    //int  foundMoves[4][8];  //1st dim=foundation number, 2nd dim=possible moves from foundation
    /**********************************************************************************************/
} STATE;

STATE* tree = NULL;

void CheckOrderedSlots(STATE *sptr)
{
    for (int ss=0; ss < 8; ss++)
    {
        if (sptr->slot[ss].count > 0)
        {
            int cardcnt = sptr->slot[ss].count;
            RANK rr = KING;
            bool ordered = true;
            for (int cc = 0; cc < cardcnt; cc++)
            {
                if (sptr->slot[ss].mycards[cc].myrank == rr)
                    rr = (RANK)((int)rr - 1);
                else
                {
                    ordered = false;
                    break;
                }
            }
            if (ordered)
                sptr->ordered_slot[ss] = true;
        }
    }
}

void PrintCard(CARD mycard)
{
    switch (mycard.myrank) {
        case ACE:
		printf(" A");
		break;
	case TWO:
		printf(" 2");
		break;
	case THREE:
		printf(" 3");
		break;
	case FOUR:
		printf(" 4");
		break;
	case FIVE:
		printf(" 5");
		break;
	case SIX:
		printf(" 6");
		break;
	case SEVEN:
		printf(" 7");
		break;
	case EIGHT:
		printf(" 8");
		break;
	case NINE:
		printf(" 9");
		break;
	case TEN:
		printf("10");
		break;
	case JACK:
		printf(" J");
		break;
	case QUEEN:
		printf(" Q");
		break;
	case KING:
		printf(" K");
		break;
    }

    switch (mycard.mysuit) {
	case CLUB:
		printf("c");
		break;
	case DIAMOND:
		printf("d");
		break;
	case HEART:
		printf("h");
		break;
	case SPADE:
		printf("s");
		break;
    }
}


void PrintSlots(STATE *stateptr)
{
    CARD tempfound;
    int cc;
    for (int ss=0; ss <= 3; ss++)
    {
        printf("s%d: ",ss);
        for (cc=0; cc < stateptr->slot[ss].count; cc++)
        {
            PrintCard(stateptr->slot[ss].mycards[cc]);
            printf(" ");
        }

        int blankcount = MAXSLOTCOUNT - stateptr->slot[ss].count;
        for (cc=0; cc < blankcount; cc++)
            printf("    ");

        tempfound.myrank = stateptr->foundation[ss];
        tempfound.mysuit = (SUIT)ss;
        printf("    ");
        PrintCard(tempfound);
        printf("       ");

        int xx = ss+4;
        printf("s%d: ",xx);
	for (cc=0; cc < stateptr->slot[xx].count; cc++)
        {
	    PrintCard(stateptr->slot[xx].mycards[cc]);
            printf(" ");
        }

	printf("\n");
    }
    printf("ordered ");
    for (int ss=0; ss < 8; ss++)
        printf("s%d:%d ",ss,stateptr->ordered_slot[ss]);
    printf("\n");
}

void PrintPossibleMoves(STATE *stateptr)
{
    for (int rr=(int)TWO; rr <= (int)KING; rr++)
    {
        switch (rr)
        {
	    case TWO:
		printf(" 2");
		break;
	    case THREE:
		printf(" 3");
		break;
	    case FOUR:
		printf(" 4");
		break;
	    case FIVE:
		printf(" 5");
		break;
	    case SIX:
		printf(" 6");
		break;
	    case SEVEN:
		printf(" 7");
		break;
	    case EIGHT:
		printf(" 8");
		break;
	    case NINE:
		printf(" 9");
		break;
	    case TEN:
		printf("10");
		break;
	    case JACK:
		printf(" J");
		break;
	    case QUEEN:
		printf(" Q");
		break;
	    case KING:
		printf(" K");
		break;
        }

        CARD cc;
        cc.myrank = (RANK)rr;
        printf(" found_cnt=%d", stateptr->foundMoveCount[rr]);
        for (int mm=0; mm < stateptr->foundMoveCount[rr]; mm++)
        {
            if (stateptr->foundMoveChild[rr][mm] == NULL)
                printf(" X");
            else
                printf("  ");
            cc.mysuit = stateptr->foundMoveSuit[rr][mm];
            PrintCard(cc);
            printf(".s%d", stateptr->foundMoveSrc[rr][mm]); 
        }
        for (int bb=0; bb < (MAX_MOVE - stateptr->foundMoveCount[rr]); bb++)
            printf("        ");

        printf(" slot_cnt=%d", stateptr->slotMoveCount[rr]);
        for (int mm=0; mm < stateptr->slotMoveCount[rr]; mm++)
        {
            if (stateptr->slotMoveChild[rr][mm] == NULL)
                printf(" X");
            else
                printf("  ");
            cc.mysuit = stateptr->slotMoveSuit[rr][mm];
            PrintCard(cc);
            printf(".s%d->s%d", stateptr->slotMoveSrc[rr][mm], stateptr->slotMoveDest[rr][mm]); 
        }
        for (int bb=0; bb < (MAX_MOVE - stateptr->slotMoveCount[rr]); bb++)
            printf("            ");

        printf(" hole_cnt=%d", stateptr->holeMoveCount[rr]);
        for (int mm=0; mm < stateptr->holeMoveCount[rr]; mm++)
        {
            if (stateptr->holeMoveChild[rr][mm] == NULL)
                printf(" X");
            else
                printf("  ");
            cc.mysuit = stateptr->holeMoveSuit[rr][mm];
            PrintCard(cc);
            printf(".s%d->s%d", stateptr->holeMoveSrc[rr][mm], stateptr->holeMoveDest[rr][mm]); 
        }
        for (int bb=0; bb < (MAX_MOVE - stateptr->holeMoveCount[rr]); bb++)
            printf("            ");

        printf("\n");
    }
    printf("level=%d\n", stateptr->level);
    if (stateptr->parent_move_type == MOVE_FOUND)
        printf("MOVE_FOUND ");
    else if (stateptr->parent_move_type == MOVE_SLOT)
        printf("MOVE_SLOT ");
    else
        printf("MOVE_HOLE ");

    PrintCard(stateptr->parent_move_card);

    printf(" parent_move_index=%d\n", stateptr->parent_move_index);
}

STATE *MakeMove(STATE *stateptr, int *statusptr)
{
    if (stateptr->foundation[CLUB] == KING &&
        stateptr->foundation[DIAMOND] == KING &&
        stateptr->foundation[HEART] == KING &&
        stateptr->foundation[SPADE] == KING)
    {
        *statusptr = 1; //win
        return stateptr;
    }

    //when we go down or up a state's child branch, 
    //the xxxxMoveCounts never change (permanent), 
    //but we NULL the child when the branch dies

    for (int rr=(int)TWO; rr <= (int)KING; rr++)
    {
        for (int mm=0; mm < stateptr->foundMoveCount[rr]; mm++)
        {
            if (stateptr->foundMoveChild[rr][mm] != NULL)
            {
                ((STATE *)stateptr->foundMoveChild[rr][mm])->parent_move_type        = MOVE_FOUND;
                ((STATE *)stateptr->foundMoveChild[rr][mm])->parent_move_card.myrank = (RANK)rr;
                ((STATE *)stateptr->foundMoveChild[rr][mm])->parent_move_card.mysuit = stateptr->foundMoveSuit[rr][mm];
                ((STATE *)stateptr->foundMoveChild[rr][mm])->parent_move_index       = mm;
                *statusptr = 0; //continue, go down this child
                return (STATE *)stateptr->foundMoveChild[rr][mm];
            }
        }
    }

    for (int rr=(int)KING; rr >= (int)TWO; rr--)
    {
        for (int mm=0; mm < stateptr->slotMoveCount[rr]; mm++)
        {
            if (stateptr->slotMoveChild[rr][mm] != NULL)
            {
                ((STATE *)stateptr->slotMoveChild[rr][mm])->parent_move_type        = MOVE_SLOT;
                ((STATE *)stateptr->slotMoveChild[rr][mm])->parent_move_card.myrank = (RANK)rr;
                ((STATE *)stateptr->slotMoveChild[rr][mm])->parent_move_card.mysuit = stateptr->slotMoveSuit[rr][mm];
                ((STATE *)stateptr->slotMoveChild[rr][mm])->parent_move_index       = mm;
                *statusptr = 0; //continue, go down this child
                return (STATE *)stateptr->slotMoveChild[rr][mm];
            }
        }
    }

    
    for (int rr=(int)KING; rr >= (int)TWO; rr--)
    {
        for (int mm=0; mm < stateptr->holeMoveCount[rr]; mm++)
        {
            if (stateptr->holeMoveChild[rr][mm] != NULL)
            {
                ((STATE *)stateptr->holeMoveChild[rr][mm])->parent_move_type        = MOVE_HOLE;
                ((STATE *)stateptr->holeMoveChild[rr][mm])->parent_move_card.myrank = (RANK)rr;
                ((STATE *)stateptr->holeMoveChild[rr][mm])->parent_move_card.mysuit = stateptr->holeMoveSuit[rr][mm];
                ((STATE *)stateptr->holeMoveChild[rr][mm])->parent_move_index       = mm;
                *statusptr = 0; //continue, go down this child

                if (rr == (int)KING)
                    ((STATE *)stateptr->holeMoveChild[rr][mm])->ordered_slot[ stateptr->holeMoveDest[rr][mm] ] = true;

                return (STATE *)stateptr->holeMoveChild[rr][mm];
            }
        }
    }
    
    if (stateptr->parent == NULL)
    {
        *statusptr = -1; //lose
        return NULL;
    }

    *statusptr = 2; //go up to parent and the child branch becomes NULL
    STATE *pp = (STATE *)(stateptr->parent);

    printf("DEBUG POP\n");
    printf("*********\n");
    if (stateptr->parent_move_type == MOVE_FOUND)
        printf("MOVE_FOUND ");
    else if (stateptr->parent_move_type == MOVE_SLOT)
        printf("MOVE_SLOT ");
    else
        printf("MOVE_HOLE ");
    PrintCard(stateptr->parent_move_card);
    printf(" parent_move_index=%d\n", stateptr->parent_move_index);
    if (stateptr->parent_move_type == MOVE_FOUND)
    {
    }
    else if (stateptr->parent_move_type == MOVE_SLOT)
    {
    }
    else
    {
        printf("pp holeMoveSrc[%d][%d]=%d\n", stateptr->parent_move_card.myrank, stateptr->parent_move_index, pp->holeMoveSrc[stateptr->parent_move_card.myrank][stateptr->parent_move_index]);
        printf("pp holeMoveDest[%d][%d]=%d\n", stateptr->parent_move_card.myrank, stateptr->parent_move_index, pp->holeMoveDest[stateptr->parent_move_card.myrank][stateptr->parent_move_index]);
        printf("pp holeMoveSuit[%d][%d]=%d\n", stateptr->parent_move_card.myrank, stateptr->parent_move_index, pp->holeMoveSuit[stateptr->parent_move_card.myrank][stateptr->parent_move_index]);
    }


    if (stateptr->parent_move_type == MOVE_FOUND)
        pp->foundMoveChild[stateptr->parent_move_card.myrank][stateptr->parent_move_index] = NULL;
    else if (stateptr->parent_move_type == MOVE_SLOT)
        pp->slotMoveChild[stateptr->parent_move_card.myrank][stateptr->parent_move_index] = NULL;
    else
        pp->holeMoveChild[stateptr->parent_move_card.myrank][stateptr->parent_move_index] = NULL;
    free(stateptr);
    return pp;
}

bool BirthChild(STATE *stateptr, int ss, int dest, CARD srcCard, MOVE_TYPE movetype)
{
    RANK rr         = srcCard.myrank;
    SUIT suit       = srcCard.mysuit;
    STATE *newchild = NULL;

    if (movetype == MOVE_SLOT)
    {
        int mm = stateptr->slotMoveCount[rr];

        stateptr->slotMoveSrc[rr][mm]   = ss;
        stateptr->slotMoveDest[rr][mm]  = dest;
        stateptr->slotMoveSuit[rr][mm]  = suit;
        stateptr->slotMoveChild[rr][mm] = (STATE *)malloc(sizeof(STATE));
        if (stateptr->slotMoveChild[rr][mm] == NULL)
            return false;

        newchild = (STATE *)stateptr->slotMoveChild[rr][mm];
        stateptr->slotMoveCount[rr] += 1;
    }
    else if (movetype == MOVE_HOLE)
    {
        int mm = stateptr->holeMoveCount[rr];

        stateptr->holeMoveSrc[rr][mm]   = ss;
        stateptr->holeMoveDest[rr][mm]  = dest;
        stateptr->holeMoveSuit[rr][mm]  = suit;
        stateptr->holeMoveChild[rr][mm] = (STATE *)malloc(sizeof(STATE));
        if (stateptr->holeMoveChild[rr][mm] == NULL)
            return false;

        newchild = (STATE *)stateptr->holeMoveChild[rr][mm];
        stateptr->holeMoveCount[rr] += 1;
    }
    else
    {
        int mm = stateptr->foundMoveCount[rr];

        stateptr->foundMoveSrc[rr][mm]  = ss;
        stateptr->foundMoveSuit[rr][mm] = suit;
        stateptr->foundMoveChild[rr][mm] = (STATE *)malloc(sizeof(STATE));
        if (stateptr->foundMoveChild[rr][mm] == NULL)
            return false;

        newchild = (STATE *)stateptr->foundMoveChild[rr][mm];
        stateptr->foundMoveCount[rr] += 1;
    }

    newchild->parent = stateptr;
    newchild->level  = stateptr->level + 1;

    for (int suit = (int)CLUB; suit <= (int)SPADE; suit++)
        newchild->foundation[suit] = stateptr->foundation[suit];

    for (int xx=0; xx < 8; xx++)
    {
        newchild->ordered_slot[xx] = stateptr->ordered_slot[xx];
        newchild->slot[xx].count   = stateptr->slot[xx].count;
        for (int cc=0; cc < newchild->slot[xx].count; cc++)
            newchild->slot[xx].mycards[cc] = stateptr->slot[xx].mycards[cc];
    }

    if (dest == GOTO_FOUNDATION)
        newchild->foundation[srcCard.mysuit] = srcCard.myrank;
    else
    {
        newchild->slot[dest].mycards[ newchild->slot[dest].count ] = srcCard;
        newchild->slot[dest].count += 1;
    }
    newchild->slot[ss].count -= 1;

    return true;
}

bool CalculatePossibleMoves(STATE *stateptr)
{
    for (int rr=(int)TWO; rr <= (int)KING; rr++)
    {
        stateptr->slotMoveCount[rr]  = 0;
        stateptr->holeMoveCount[rr]  = 0;
        stateptr->foundMoveCount[rr] = 0;
    }

    bool malloc_ok = true;
    for (int ss=0; ss < 8; ss++)
    {
        if (stateptr->slot[ss].count > 0)
	{
		CARD srcCard = stateptr->slot[ss].mycards[ stateptr->slot[ss].count - 1 ];
                RANK srcRank = srcCard.myrank;
		SUIT srcSuit = srcCard.mysuit;

		if (stateptr->foundation[srcSuit] == (srcRank - 1))
		{
                        malloc_ok = BirthChild(stateptr, ss, GOTO_FOUNDATION, srcCard, MOVE_FOUND);
                        if (malloc_ok == false)
                            return false;
		}

                if (srcRank != TWO)
                {
        	    for (int dd=0; dd < 8; dd++)
		    {
			if (dd != ss)
			{
				if (stateptr->slot[dd].count > 0) //destination slot has a card
				{
					CARD destCard = stateptr->slot[dd].mycards[ stateptr->slot[dd].count - 1 ];	
					RANK destRank = destCard.myrank;
                                        if ((destRank - 1) == srcRank)
					{
                                            malloc_ok = BirthChild(stateptr, ss, dd, srcCard, MOVE_SLOT);
                                            if (malloc_ok == false)
                                                return false;
					}
				}
				else //destination slot is empty
				{
                                        malloc_ok = BirthChild(stateptr, ss, dd, srcCard, MOVE_HOLE);
                                        if (malloc_ok == false)
                                            return false;
				}
			}
		    }
		}
	}
    }
    return true;
}


void TranslateTextToCard(char RR, char SS, CARD *cardptr)
{
    switch (RR) {
	case '1':
		cardptr->myrank = TEN;
		break;
	case '2':
		cardptr->myrank = TWO;
		break;
	case '3':
		cardptr->myrank = THREE;
		break;
	case '4':
		cardptr->myrank = FOUR;
		break;
	case '5':
		cardptr->myrank = FIVE;
		break;
	case '6':
		cardptr->myrank = SIX;
		break;
	case '7':
		cardptr->myrank = SEVEN;
		break;
	case '8':
		cardptr->myrank = EIGHT;
		break;
	case '9':
		cardptr->myrank = NINE;
		break;
	case 'J':
		cardptr->myrank = JACK;
		break;
	case 'Q':
		cardptr->myrank = QUEEN;
		break;
	case 'K':
		cardptr->myrank = KING;
		break;
	default:
		cardptr->myrank = ACE;
		break;
    }

    switch (SS) {
	case 'c':
		cardptr->mysuit = CLUB;
		break;
	case 'd':
		cardptr->mysuit = DIAMOND;
		break;
	case 'h':
		cardptr->mysuit = HEART;
		break;
	default:
		cardptr->mysuit = SPADE;
		break;
    }
}

void FillSlot(STATE *stateptr, int slotindex, CARD *cardptr, int cardcount)
{
    stateptr->slot[slotindex].count = cardcount;
    for (int cc=0; cc < cardcount; cc++)
    {
        stateptr->slot[slotindex].mycards[cc] = cardptr[cc];
    }
}

bool ReadStartOfGame()
{
    int ii;
    char  R1, S1, R2, S2, R3, S3, R4, S4, R5, S5, R6, S6;
    CARD  card[6];

    tree = (STATE *)malloc(sizeof(STATE));
    if (tree == NULL)
        return false;

    tree->level  = 0;
    tree->parent = NULL;
    tree->parent_move_type        = MOVE_FOUND;
    tree->parent_move_card.myrank = ACE;
    tree->parent_move_card.mysuit = CLUB;
    tree->parent_move_index       = -1;

    tree->foundation[CLUB]    = ACE;
    tree->foundation[SPADE]   = ACE;
    tree->foundation[HEART]   = ACE;
    tree->foundation[DIAMOND] = ACE;

    FILE *fptr = fopen("start.txt","r");
    for (ii=0; ii<8; ii++)
    {
        fscanf(fptr,"%c%c,%c%c,%c%c,%c%c,%c%c,%c%c\n",&R1,&S1,&R2,&S2,&R3,&S3,&R4,&S4,&R5,&S5,&R6,&S6);

        TranslateTextToCard(R1,S1,&(card[0]));
        TranslateTextToCard(R2,S2,&(card[1]));
        TranslateTextToCard(R3,S3,&(card[2]));
        TranslateTextToCard(R4,S4,&(card[3]));
        TranslateTextToCard(R5,S5,&(card[4]));
        TranslateTextToCard(R6,S6,&(card[5]));

        FillSlot(tree, ii, card, 6);
    }
    fclose(fptr);

    return true;
}


void EliminateLastCardMovedLoop(STATE *sptr)
{
    RANK rr   = sptr->parent_move_card.myrank;
    SUIT suit = sptr->parent_move_card.mysuit;
    
    if (sptr->slotMoveCount[rr] > 0)
    {
        for (int mm=0; mm < sptr->slotMoveCount[rr]; mm++)
        {
            if (sptr->slotMoveSuit[rr][mm] == suit)
            {
                if (sptr->slotMoveChild[rr][mm] != NULL)
                {
                    free((STATE *)(sptr->slotMoveChild[rr][mm]));
                    sptr->slotMoveChild[rr][mm] = NULL;
                }
            }
        }
    }

    if (sptr->holeMoveCount[rr] > 0)
    {
        for (int mm=0; mm < sptr->holeMoveCount[rr]; mm++)
        {
            if (sptr->holeMoveSuit[rr][mm] == suit)
            {
                if (sptr->holeMoveChild[rr][mm] != NULL)
                {
                    free((STATE *)(sptr->holeMoveChild[rr][mm]));
                    sptr->holeMoveChild[rr][mm] = NULL;
                }
            }
        }
    }
}

void EliminateWorthlessSlotMoves(STATE *sptr)
{
    for (int rr = (int)QUEEN; rr >= (int)TWO; rr--)
    {
        if (sptr->slotMoveCount[rr] > 0)
        {
            for (int mm=0; mm < sptr->slotMoveCount[rr]; mm++)
            {
                int ss = sptr->slotMoveSrc[rr][mm];
                if (sptr->ordered_slot[ss] == true)
                {
                    free((STATE *)(sptr->slotMoveChild[rr][mm]));
                    sptr->slotMoveChild[rr][mm] = NULL;
                }
                else if (sptr->slot[ss].count >= 2)
                {
                    CARD pc = sptr->slot[ss].mycards[ sptr->slot[ss].count - 2 ];
                    if ((pc.myrank - 1) == rr)
                    {
                        int dest = sptr->slotMoveDest[rr][mm];
                        if (sptr->ordered_slot[dest] == false)
                        {
                            if (sptr->slotMoveChild[rr][mm] != NULL)
                            {
                                free((STATE *)(sptr->slotMoveChild[rr][mm]));
                                sptr->slotMoveChild[rr][mm] = NULL;
                            }
                        }
                    }
                }
            }
        }
    }
}

void CheckStupidMove(STATE *sptr, RANK rr, int maxcardcount)
{
    if (sptr->holeMoveCount[rr] > 0)
    {
        for (int mm=0; mm < sptr->holeMoveCount[rr]; mm++)
        {
            int ss = sptr->holeMoveSrc[rr][mm];
            if (sptr->slot[ss].count == maxcardcount) //stupid worthless move
            {
                if (maxcardcount == 1)
                {
                    if (sptr->holeMoveChild[rr][mm] != NULL)
                    {
                        free((STATE *)(sptr->holeMoveChild[rr][mm]));
                        sptr->holeMoveChild[rr][mm] = NULL;
                    }
                }
                else
                {
                    bool goodorder = true;
                    RANK checkrank = KING;
                    for (int cc=0; cc < (maxcardcount - 1); cc++)
                    {
                        if (sptr->slot[ss].mycards[cc].myrank != checkrank)
                        {
                            goodorder = false;
                            break;
                        }
                        else
                            checkrank = (RANK)((int)checkrank - 1);
                    }

                    if (goodorder)
                    {
                        if (sptr->holeMoveChild[rr][mm] != NULL)
                        {
                            free((STATE *)(sptr->holeMoveChild[rr][mm]));
                            sptr->holeMoveChild[rr][mm] = NULL;
                        }
                    }
                }
            }
        }
    }
}

void EliminateStupidMovesToHole(STATE *sptr)
{
    CheckStupidMove(sptr, KING,  1);
    CheckStupidMove(sptr, QUEEN, 2);
    CheckStupidMove(sptr, JACK,  3);
    CheckStupidMove(sptr, TEN,   4);
    CheckStupidMove(sptr, NINE,  5);
    CheckStupidMove(sptr, EIGHT, 6);
    CheckStupidMove(sptr, SEVEN, 7);
    CheckStupidMove(sptr, SIX,   8);
    CheckStupidMove(sptr, FIVE,  9);
    CheckStupidMove(sptr, FOUR, 10);
    CheckStupidMove(sptr, THREE,11);
    CheckStupidMove(sptr, TWO,  12);

    CheckStupidMove(sptr, QUEEN, 1);
    CheckStupidMove(sptr, JACK,  1);
    CheckStupidMove(sptr, TEN,   1);
    CheckStupidMove(sptr, NINE,  1);
    CheckStupidMove(sptr, EIGHT, 1);
    CheckStupidMove(sptr, SEVEN, 1);
    CheckStupidMove(sptr, SIX,   1);
    CheckStupidMove(sptr, FIVE,  1);
    CheckStupidMove(sptr, FOUR,  1);
    CheckStupidMove(sptr, THREE, 1);
    CheckStupidMove(sptr, TWO,   1);
}

void EliminateSameSlotMovesPreviouslyEliminated(STATE *sp)
{
    printf("helloslot\n");
    STATE *pp = (STATE *)(sp->parent);
    if (pp == NULL)
        return;

    for (int rr = (int)KING; rr >= (int)TWO; rr--)
    {
        if ((sp->slotMoveCount[rr] > 0) && (pp->slotMoveCount[rr] > 0))
        {
            for (int mm=0; mm < pp->slotMoveCount[rr]; mm++)
            {
                if (pp->slotMoveChild[rr][mm] == NULL)
                {
                    int ppsrc   = pp->slotMoveSrc[rr][mm];
                    int ppdest  = pp->slotMoveDest[rr][mm];
                    SUIT ppsuit = pp->slotMoveSuit[rr][mm];

                    for (int xx=0; xx < sp->slotMoveCount[rr]; xx++)
                    {
                        int spsrc  = sp->slotMoveSrc[rr][xx];
                        int spdest = sp->slotMoveDest[rr][xx];
                        int spsuit = sp->slotMoveSuit[rr][xx];

                        if (ppsrc == spsrc && ppdest == spdest && ppsuit == spsuit && sp->slotMoveChild[rr][xx] != NULL)
                        {
                            printf("rr=%d ppsrc=%d ppdest=%d ppsuit=%d spsrc=%d spdest=%d spsuit=%d\n", rr, ppsrc, ppdest, ppsuit, spsrc, spdest, spsuit);
                            free((STATE *)(sp->slotMoveChild[rr][xx]));
                            sp->slotMoveChild[rr][xx] = NULL;
                        }
                    }
                }
            }
        }
    }
    printf("goodbyeslot\n");
}


void EliminateSameHoleMovesPreviouslyEliminated(STATE *sp)
{
    printf("hellohole\n");
    STATE *pp = (STATE *)(sp->parent);
    if (pp == NULL)
        return;

    for (int rr = (int)KING; rr >= (int)TWO; rr--)
    {
        if ((sp->holeMoveCount[rr] > 0) && (pp->holeMoveCount[rr] > 0))
        {
            for (int mm=0; mm < pp->holeMoveCount[rr]; mm++)
            {
                if (pp->holeMoveChild[rr][mm] == NULL)
                {
                    int ppsrc   = pp->holeMoveSrc[rr][mm];
                    int ppdest  = pp->holeMoveDest[rr][mm];
                    SUIT ppsuit = pp->holeMoveSuit[rr][mm];

                    for (int xx=0; xx < sp->holeMoveCount[rr]; xx++)
                    {
                        int spsrc  = sp->holeMoveSrc[rr][xx];
                        int spdest = sp->holeMoveDest[rr][xx];
                        int spsuit = sp->holeMoveSuit[rr][xx];

                        if (ppsrc == spsrc && ppdest == spdest && ppsuit == spsuit && sp->holeMoveChild[rr][xx] != NULL)
                        {
                            printf("rr=%d ppsrc=%d ppdest=%d ppsuit=%d spsrc=%d spdest=%d spsuit=%d\n", rr, ppsrc, ppdest, ppsuit, spsrc, spdest, spsuit);
                            free((STATE *)(sp->holeMoveChild[rr][xx]));
                            sp->holeMoveChild[rr][xx] = NULL;
                        }
                    }
                }
            }
        }
    }
    printf("goodbyehole\n");
}


void EliminateOneFoundationMoveWorthlessSlotMovesNoHoles(STATE *sp)
{
    //already know hole count is 0
    //(1)have foundation move with only 1 rank
    //(2)foundation move cards are not only cards in slot (so when you move them to foundation, then no holes are created)
    //(3)no lower cards than foundation move cards in any other slot able to move

    //if above holds true, then we can kill all slot moves for that rank and higher ranks

    int  totalranks = 0;
    RANK checkrank  = ACE;
    int  checkcount = 0;
    for (int rr = (int)TWO; rr <= (int)KING; rr++)
    {
        if (sp->foundMoveCount[rr] > 0)
        {
            if (checkrank == ACE)
            {
                checkrank = (RANK)rr;
                checkcount = sp->foundMoveCount[rr];
            }
            totalranks++;
        }
    }

    bool foundlower = false;
    if (checkrank != ACE && totalranks == 1)
    {
        if (checkrank == TWO)
            foundlower = false;
        else
        {
            for (int xx = (int)TWO; xx < (int)checkrank; xx++)
            {
                if (sp->slotMoveCount[xx] > 0)
                {
                    foundlower = true;
                    break;
                }
            }
        }
    }
    else
    {
        return;
    }

    if (foundlower == false)
    {
        bool only_card_exists = false;
        for (int cc=0; cc < checkcount; cc++)
        {
            int ss = sp->foundMoveSrc[checkrank][cc];
            if (sp->slot[ss].count == 1)
            {
                only_card_exists = true;
                break;
            }
        }

        if (only_card_exists == false)
        {
            for (int cc=0; cc < checkcount; cc++)
            {
                SUIT suit = sp->foundMoveSuit[checkrank][cc];
                for (int mm=0; mm < sp->slotMoveCount[checkrank]; mm++)
                {
                    if (sp->slotMoveSuit[checkrank][mm] == suit && sp->slotMoveChild[checkrank][mm] != NULL)
                    {
                        free((STATE *)(sp->slotMoveChild[checkrank][mm]));
                        sp->slotMoveChild[checkrank][mm] = NULL;
                    }
                }
            }

            for (int rr=((int)checkrank + 1); rr <= (int)KING; rr++)
            {
                for (int mm=0; mm < sp->slotMoveCount[rr]; mm++)
                {
                    if (sp->slotMoveChild[rr][mm] != NULL)
                    {
                        free((STATE *)(sp->slotMoveChild[rr][mm]));
                        sp->slotMoveChild[rr][mm] = NULL;
                    }
                }
            }
        }
    }
}

bool Eliminate2HoleLoop(STATE *sptr)
{
    //we already know we got 2 holes
    int hole1 = -1;
    int hole2 = -1;
    int goodslot[6];
    int goodcount[6];
    int index = 0;
    for (int ss=0; ss < 8; ss++)
    {
        if (sptr->slot[ss].count == 0)
        {
            if (hole1 == -1)
                hole1 = ss;
            else
                hole2 = ss;
        }
        else
        {
            goodslot[index] = ss;
            goodcount[index] = sptr->slot[ss].count;
            index++;
        }
    }

    //now go thru goodslots (they have cards)
    //EliminateWorthlessSlotMoves takes care of the 1st card already
    bool eliminate = true;
    for (int index=0; index < 6; index++)
    {
        int gs = goodslot[index];
        int gc = goodcount[index];

        CARD card1 = sptr->slot[gs].mycards[gc - 1];
        CARD card2 = sptr->slot[gs].mycards[gc - 1];
        CARD card3 = sptr->slot[gs].mycards[gc - 1];
        CARD card4 = sptr->slot[gs].mycards[gc - 1];

        //2nd card
        if (gc >= 2)
        {
            card2 = sptr->slot[gs].mycards[gc - 2];
            for (int dd=0; dd < 6; dd++)
            {
                if (dd != index)
                {
                    int ds = goodslot[dd];
                    int dc = goodcount[dd];
                    CARD dcard = sptr->slot[ds].mycards[dc - 1];
                    if ((dcard.myrank - 1) == card2.myrank)
                    {
                        if (gc >= 3)
                        {
                            card3 = sptr->slot[gs].mycards[gc - 3];
                            if ((card3.myrank - 1) != card2.myrank)
                            {
                                eliminate = false;
                                break;
                            }
                        }
                        else
                        {
                            eliminate = false;
                            break;
                        }
                    }
                    else if (dcard.myrank == (card2.myrank - 1))
                    {
                        if (dc >= 2)
                        {
                            CARD prev_dcard = sptr->slot[ds].mycards[dc - 2];
                            if (prev_dcard.myrank != card2.myrank)
                            {
                                eliminate = false;
                                break;
                            }
                        }
                        else
                        {
                            eliminate = false;
                            break;
                        }
                    }
                }
            }
            if (eliminate == false)
                break;

            if ((card1.myrank - 1) == card2.myrank)
            {
                printf("card1=");
                PrintCard(card1);
                printf(" card2=");
                PrintCard(card2);
                printf("\n");

                eliminate = false;
                break;
            }
        }

        //3rd card
        if (gc >= 3)
        {
            card3 = sptr->slot[gs].mycards[gc - 3];
            for (int dd=0; dd < 6; dd++)
            {
                if (dd != index)
                {
                    int ds = goodslot[dd];
                    int dc = goodcount[dd];
                    CARD dcard = sptr->slot[ds].mycards[dc - 1];
                    if ((dcard.myrank - 1) == card3.myrank)
                    {
                        if (gc >= 4)
                        {
                            card4 = sptr->slot[gs].mycards[gc - 4];
                            if ((card4.myrank - 1) != card3.myrank)
                            {
                                eliminate = false;
                                break;
                            }
                        }
                        else
                        {
                            eliminate = false;
                            break;
                        }
                    }
                    else if (dcard.myrank == (card3.myrank - 1))
                    {
                        if (dc >= 2)
                        {
                            CARD prev_dcard = sptr->slot[ds].mycards[dc - 2];
                            if (prev_dcard.myrank != card3.myrank)
                            {
                                eliminate = false;
                                break;
                            }
                        }
                        else
                        {
                            eliminate = false;
                            break;
                        }
                    }
                }
            }
            if (eliminate == false)
                break;

            if (((card1.myrank - 1) == card3.myrank) ||
                ((card2.myrank - 1) == card3.myrank))
            {
                printf("card1=");
                PrintCard(card1);
                printf(" card2=");
                PrintCard(card2);
                printf(" card3=");
                PrintCard(card3);
                printf("\n");

                eliminate = false;
                break;
            }
        }

        //4th card only if last 2 cards are ordered
        if (gc >= 4 && ((card2.myrank - 1) == card1.myrank))
        {
            card4 = sptr->slot[gs].mycards[gc - 4];
            for (int dd=0; dd < 6; dd++)
            {
                if (dd != index)
                {
                    int ds = goodslot[dd];
                    int dc = goodcount[dd];
                    CARD dcard = sptr->slot[ds].mycards[dc - 1];
                    if ((dcard.myrank - 1) == card4.myrank)
                    {
                        if (gc >= 5)
                        {
                            CARD card5 = sptr->slot[gs].mycards[gc - 5];
                            if ((card5.myrank - 1) != card4.myrank)
                            {
                                eliminate = false;
                                break;
                            }
                        }
                        else
                        {
                            eliminate = false;
                            break;
                        }
                    }
                    else if (dcard.myrank == (card4.myrank - 1))
                    {
                        if (dc >= 2)
                        {
                            CARD prev_dcard = sptr->slot[ds].mycards[dc - 2];
                            if (prev_dcard.myrank != card4.myrank)
                            {
                                eliminate = false;
                                break;
                            }
                        }
                        else
                        {
                            eliminate = false;
                            break;
                        }
                    }
                }
            }
            if (eliminate == false)
                break;

            if (((card1.myrank - 1) == card4.myrank) ||
                ((card2.myrank - 1) == card4.myrank) ||
                ((card3.myrank - 1) == card4.myrank))
            {
                printf("card1=");
                PrintCard(card1);
                printf(" card2=");
                PrintCard(card2);
                printf(" card3=");
                PrintCard(card3);
                printf(" card4=");
                PrintCard(card4);

                eliminate = false;
                break;
            }
        }
    }

    if (eliminate)
    {
        for (int rr=(int)KING; rr >= (int)TWO; rr--)
        {
            for (int mm=0; mm < sptr->holeMoveCount[rr]; mm++)
            {
                if (sptr->holeMoveChild[rr][mm] != NULL)
                {
                    free((STATE *)(sptr->holeMoveChild[rr][mm]));
                    sptr->holeMoveChild[rr][mm] = NULL;
                }
            }
        }
    }

    return eliminate;
}

int AI_Logic(STATE *stateptr)
{
    bool did_eliminate = false;

    CheckOrderedSlots(stateptr);
    PrintSlots(stateptr);

    bool malloc_ok = CalculatePossibleMoves(stateptr);
    if (malloc_ok == false)
    {
        printf("FAILED CalculatePossibleMoves !!!!!\n");
        return -2; //error
    }

    int holes = 0;
    for (int ss=0; ss < 8; ss++)
    {
        if (stateptr->slot[ss].count == 0)
            holes++;
    }

    if (holes == 0) //no holes
    {
        EliminateLastCardMovedLoop(stateptr);
        EliminateWorthlessSlotMoves(stateptr);
        EliminateSameSlotMovesPreviouslyEliminated(stateptr);
        EliminateSameHoleMovesPreviouslyEliminated(stateptr);
        EliminateOneFoundationMoveWorthlessSlotMovesNoHoles(stateptr);
    }
    else //1 or more holes exist
    {
        EliminateLastCardMovedLoop(stateptr);
        EliminateWorthlessSlotMoves(stateptr);
        EliminateStupidMovesToHole(stateptr);
        EliminateSameSlotMovesPreviouslyEliminated(stateptr);
        EliminateSameHoleMovesPreviouslyEliminated(stateptr);

        if (holes == 2)
            did_eliminate = Eliminate2HoleLoop(stateptr);
    }

    PrintPossibleMoves(stateptr);

    if (did_eliminate)
    {
        printf("did_eliminate\n");
        int dummy;
//        scanf("%d", &dummy);
    }

    if (holes == 2)
        return 3;

    return 0;
}


int main()
{
    bool malloc_ok = ReadStartOfGame();
    if (malloc_ok == false)
    {
        printf("FAILED ReadStartOfGame !!!!!\n");
        return 0;
    }
    CheckOrderedSlots(tree);
    PrintSlots(tree);

    malloc_ok = CalculatePossibleMoves(tree);
    if (malloc_ok == false)
    {
        printf("FAILED CalculatePossibleMoves(tree) !!!!!\n");
        return 0;
    }
    PrintPossibleMoves(tree);
    printf("========================================================================================================================================================\n");
    int dummy;

    
    int movenum = 1;
    STATE *stateptr = tree;
    int status = 0; /***** 1=win, -1=lose, -2=error, 0=continue 2=go up 3=found 2 holes *****/
    while (stateptr != NULL && (status==0 || status==2 || status==3))
    {
        stateptr = MakeMove(stateptr, &status);
        if (status != 0 && status != 2 && status != 3)
            break;

        if (status == 2)
        {
            printf("--------------------\n");
            printf("----- GOING UP -----\n");
            printf("--------------------\n");
            PrintSlots(stateptr);
            PrintPossibleMoves(stateptr);
        }
        else
        {
            printf("movenum=%d\n",movenum);
            movenum++;
            status = AI_Logic(stateptr);
        }
        printf("======================================================================================================================================================\n");
        if (movenum >= 37287)
        {
            status = -2;
            break;
        }
    }
    if (status == 1)
       printf("WIN.\n");
    else if (status == -1)
       printf("LOSE.\n");
    else
       printf("ERROR.\n");
    return 0;
}

