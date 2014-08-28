/*Cameron Bates
  COP3502C - C014
  Assignment #2 - Knights Mart
  June 8, 2011
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct KnightsMartProduct {
    int itemNum;
    char itemName[21];
    double unitPrice;
    int stockQty;
    int restockQty;
    struct KnightsMartProduct *next;
} KMProduct;

typedef struct KnightsMartRestockProduct {
    int itemNum;
    struct KnightsMartRestockProduct *next;
} KMRestockProduct;

typedef struct KnightsMartSale {
    char firstName[21];
    char lastName[21];
    int numItemsOnList;  // # of items on shopping list
    int *itemsPurchased; // array of item numbers
    struct KnightsMartSale *next;
} KMSale;

KMProduct* addItem(KMProduct* list); //Prototype for function that adds items to the inventory
void restock(KMRestockProduct** rsList, KMProduct* prList); //Restocks items that are in the Restock linked list
void inventory(KMProduct* list); //prints out current inventory
void customer(KMProduct* prodList, KMRestockProduct** stockList, KMSale** saleList); //This function does a lot of stuff
void printsum(); //Prints a summary of the days events

void freeProd(KMProduct* list); //frees the KMProducts list
void freeStock(KMRestockProduct* list); //Frees the KMRestock list
void freeSale(KMSale* list); //frees the KMSales list

FILE *fin, *fout; //Global file pointers. Simplifies my life.

int main()
{
    //variable initialization
    int numDays, commNum;
    int i, x;
    KMProduct* KMProducts = NULL;
    KMRestockProduct* KMRestockList = NULL;
    KMSale* KMSales = NULL;
    char command[15];
    fin = fopen("KnightsMart.in","r");
    fout = fopen("KnightsMart.out","w");

    fscanf(fin, "%d", &numDays); //How many days the program runs

    for(i=0;i<numDays;i++) //For loop handles the number of days the program runs for
    {
        fprintf(fout, "*************************\n"
               "UCF KnightsMart Day %d\n"
               "*************************\n", i+1);

        fscanf(fin, "%d", &commNum); //How many commands are run for the current day

        for(x=0; x<commNum; x++)//For loop handles number of commands performed
        {
            fscanf(fin, "%s", command); //Current command

            //if statements determine which function should be called
            if(!strcmp(command, "ADDITEM"))
                KMProducts = addItem(KMProducts);

            if(!strcmp(command, "RESTOCK"))
                restock(&KMRestockList, KMProducts);

            if(!strcmp(command, "CUSTOMER"))
                customer(KMProducts, &KMRestockList, &KMSales);

            if(!strcmp(command, "INVENTORY"))
                inventory(KMProducts);

            if(!strcmp(command, "PRINTDAYSUMMARY"))
                printsum(&KMSales, KMProducts);

        }
    }

    fclose(fin); //Close input and output files
    fclose(fout);
    freeProd(KMProducts); freeStock(KMRestockList); freeSale(KMSales); //free allocated memory

    return 0;
}

KMProduct* addItem(KMProduct* list)
{
    //Initialization of new product node and
    //filling in the values for that node
    KMProduct* help_ptr = list;
    KMProduct* pNew = (KMProduct*)malloc(sizeof(KMProduct));
    fscanf(fin, "%d", &pNew->itemNum);
    fscanf(fin, "%s", pNew->itemName);
    fscanf(fin, "%lf", &pNew->unitPrice);
    fscanf(fin, "%d", &pNew->stockQty);
    fscanf(fin, "%d", &pNew->restockQty);
    pNew->next = NULL;

    //If the list is null or the item number of the new node
    //is lower than the current first node, set the new node
    //as the first node
    if((list == NULL) || (list->itemNum > pNew->itemNum))
    {
        pNew->next = list;
        list = pNew;

        fprintf(fout, "Item %d, %s, with a cost of $%.2lf and initial stock of %d, has been added to the product database.\n",
                pNew->itemNum, pNew->itemName, pNew->unitPrice, pNew->stockQty);

        return list;
    }

    //Places the new node in the appropriate place if not the front
    while((help_ptr->next != NULL) && (help_ptr->next->itemNum < pNew->itemNum))
        help_ptr = help_ptr->next;

    pNew->next = help_ptr->next;
    help_ptr->next = pNew;

    fprintf(fout, "Item %d, %s, with a cost of $%.2lf and initial stock of %d, has been added to the product database.\n",
            pNew->itemNum, pNew->itemName, pNew->unitPrice, pNew->stockQty); //Prints out a statement that the product has been added to the inventory

    return list;
}

void restock(KMRestockProduct** rsList, KMProduct* prList)
{
    KMRestockProduct* rs_ptr = *rsList;
    KMRestockProduct* deleteNode;
    KMProduct* pr_ptr = prList;

    fprintf(fout, "RESTOCK:  ");

    if(*rsList == NULL) //Empty list prints out appropriate line
        fprintf(fout, "the Restock List contains no items.\n");
    else //If not empty, begin searching through product list
    {
        fprintf(fout, "the following items have been reordered and restocked:\n");

        while(rs_ptr!=NULL)
        {
            //search for proper product node
            while(rs_ptr->itemNum != pr_ptr->itemNum)
                pr_ptr = pr_ptr->next;

            //node found, so restock quantity
            if(rs_ptr->itemNum == pr_ptr->itemNum)
            {
                deleteNode = rs_ptr;
                rs_ptr = rs_ptr->next;
                pr_ptr->stockQty = pr_ptr->restockQty;

                fprintf(fout, "\tItem %d, %s, restocked to a quantity of %d.\n",
                        pr_ptr->itemNum, pr_ptr->itemName, pr_ptr->stockQty); //Prints out confirmation that the item was restocked

                free(deleteNode); //free memory
                pr_ptr = prList; //reset helper pointer
            }

        }
    }

    (*rsList) = NULL; //set head of list to NULL
}

void customer(KMProduct* prodList, KMRestockProduct** stockList, KMSale** saleList)
{
    int i = 0, x, buy, currBuy;
    KMSale* sl_help = *saleList;
    KMProduct* pr_help = prodList;
    KMRestockProduct* st_help = *stockList;
    KMSale* pNew = (KMSale*)malloc(sizeof(KMSale));

    //Set new node equal to the appropriate information
    fscanf(fin, "%s%s", pNew->firstName, pNew->lastName);
    fscanf(fin, "%d", &pNew->numItemsOnList);
    pNew->itemsPurchased = (int*)malloc(sizeof(int)*(pNew->numItemsOnList));
    for(x=0; x<pNew->numItemsOnList; x++){ fscanf(fin, "%d", &pNew->itemsPurchased[x]); }
    pNew->next = NULL;

    fprintf(fout, "Customer %s %s came and made ", pNew->firstName, pNew->lastName); //print out that a customer entered

    while(i < pNew->numItemsOnList)
    {
        currBuy = 0; //reset counter

        while(pr_help!=NULL)
        {
            if(pNew->itemsPurchased[i] == pr_help->itemNum) //if the item wanted is found in stock,
            {
                if(pr_help->stockQty > 0) //if there is stock for the item
                {
                    //set counters equal to 1 because item is purchased
                    buy = 1;
                    currBuy = 1;

                    if(pNew->itemsPurchased[i+1] >= pr_help->stockQty) //If the wants more than what is in stock
                    {
                        pNew->itemsPurchased[i+1] = pr_help->stockQty; //give customer all of the stock
                        pr_help->stockQty = 0; //set stock equal to 0

                        KMRestockProduct* sNew = (KMRestockProduct*)malloc(sizeof(KMRestockProduct));
                        sNew->itemNum = pr_help->itemNum;
                        sNew->next = NULL;

                        if(*stockList==NULL) //if list is empty, set new node as the front
                            st_help = sNew;
                        else //else, put the node at the end of the list
                        {
                            while(st_help->next!=NULL)
                                st_help = st_help->next;
                            st_help->next = sNew;   
                        }
                    }

                    else //More in stock than customer wants
                    {
                        pr_help->stockQty = pr_help->stockQty - pNew->itemsPurchased[i+1]; //set the stock of the item equal to the difference of what is wanted
                    }
                }
                else //else, no stock exists and 0 is set as the # purchased for that item
                {
                    if(buy==1){} //if something has already been bought, do nothing
                    else{buy = 0;} //else, set buy = 0
                    pNew->itemsPurchased[i+1] = 0; //set that items value to zero
                }

            }

            pr_help = pr_help->next; //increment help pointer
        }

        pr_help = prodList; //reset list
        if(currBuy == 0){ pNew->itemsPurchased[i+1] = 0; } //If that item was not bought, set its value to 0
        i += 2; //increment
    }

    if(buy == 1)//If sale made, add to the list
    {
        fprintf(fout, "some purchases.\n");
        if(*saleList == NULL) //if NULL, make the new node the beginning of the list
            (*saleList) = pNew;
        else
        {
            while(sl_help->next!=NULL) //Add the sale node to the end of the list
                sl_help = sl_help->next;
            sl_help->next = pNew;
        }
    }
    else
    {
        fprintf(fout, "no purchases.\n"); //if no sale was made, print that out and free the node created
        free(pNew->itemsPurchased);
        free(pNew);
    }
}

void inventory(KMProduct* list)
{
    KMProduct* help_ptr = list;

    fprintf(fout, "INVENTORY:  ");

    if(list == NULL) //if no items print appropriate message
        fprintf(fout, "contains no items.\n");
    else
    {
        fprintf(fout, "contains the following items:\n");
        while(help_ptr!=NULL)
        {
            fprintf(fout, "\t| Item %6d | %-20s | $%7.2lf | %4d unit(s) |\n",
                   help_ptr->itemNum, help_ptr->itemName, help_ptr->unitPrice, help_ptr->stockQty); //print out item information

            help_ptr = help_ptr->next; //increment pointer
        }
    }
}

void printsum(KMSale** saleList, KMProduct* prodList)
{
     KMProduct* pr_help = prodList;
     KMSale* sale_help = *saleList;
     KMSale* deleteNode;
     int i, sales = 0, x, saleNum = 0;

     double saleTot = 0, dayTot = 0;

     fprintf(fout, "KnightsMart Daily Sales report:\n");

     if(*saleList == NULL) //if no sales, print out appropriate message
          fprintf(fout, "\tThere were no sales today.\n\n");
     else
     {
          while(sale_help!=NULL)
          {
              i=0; saleTot = 0; sales = 0; //reset counters

              for(x=0; x<sale_help->numItemsOnList;x++){ sales += sale_help->itemsPurchased[x+1]; x++; }

              fprintf(fout, "\tSale #%d, %s %s purchased %d item(s):\n", (saleNum+1), sale_help->firstName, sale_help->lastName, sales); //prints customer information

              while(i<sale_help->numItemsOnList) //controls itemsPurchased
              {
                   while((pr_help!=NULL) && (sale_help->itemsPurchased[i] != pr_help->itemNum)){ pr_help = pr_help->next; } //find item sold in the items list
                   if(sale_help->itemsPurchased[i+1] == 0){ i+=2; pr_help = prodList; } //If this item wasn't bought, increment and reset list
                   else if(sale_help->itemsPurchased[i] == pr_help->itemNum)
                   {
                       fprintf(fout, "\t\t| Item %6d | %-20s | $%7.2lf (x%4d) |\n",
                               sale_help->itemsPurchased[i], pr_help->itemName, pr_help->unitPrice, sale_help->itemsPurchased[i+1]); //Print out sale information

                       saleTot += (sale_help->itemsPurchased[i+1] * pr_help->unitPrice); //get money
                   }

                   i+=2; pr_help = prodList; //reset helper pointer and increment counter
              }

              fprintf(fout, "\t\tTotal: $%.2lf\n", saleTot);//Print customer total spent
              dayTot += saleTot;

              //Free from the front of the list
              deleteNode = sale_help;
              sale_help = sale_help->next;
              saleNum++;

              free(deleteNode->itemsPurchased); //Free the int array and the node
              free(deleteNode);
          }

           fprintf(fout, "\tGrand Total: $%.2lf\n\n", dayTot); //Print total earnings for the day
     }

     (*saleList) = NULL; //Set the head of the list to NULL
}

void freeProd(KMProduct* list)
{
    KMProduct* deleteNode;

    //Go through the list and free the nodes
    while(list!=NULL)
     {
        deleteNode = list;
        list = list->next;
        free(deleteNode);
     }
}

void freeStock(KMRestockProduct* list)
{
     KMRestockProduct* deleteNode;

     //Go through the list and free the nodes
     while(list!=NULL)
     {
        deleteNode = list;
        list = list->next;
        free(deleteNode);
     }
}

void freeSale(KMSale* list)
{
     KMSale* deleteNode;

     //Go through the list and free the allocated int array and the node itself
     while(list!=NULL)
     {
        deleteNode = list;
        list = list->next;
        free(deleteNode->itemsPurchased);
        free(deleteNode);
     }
}
