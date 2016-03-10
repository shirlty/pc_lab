#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <time.h>
#include <pthread.h>

/*
* class definitions for Box
* l:leftupper X ; r:rightupper X ; t: leftupper Y ; b: leftdown Y; p:parameter
* nTop,nBottom,nLeft,nRight: the number of neighbors
* *leftN,*rightN,*topN,*bottomN: the id of neighbors
* curDsv: current DSV; newDsv: new DSV
*/
struct Box{
    int     l,r,t,b,p;
    int     nTop,nBottom,nLeft,nRight;
    int     *leftN,*rightN,*topN,*bottomN;
    float   curDsv,newDsv;
};

/*
* global variables
* *boxes: array of Box
*/
struct Box *boxes=NULL;
int numBox;
float affect_rate,epsilon;
int numThread;
int isConverge=1;
int count=0;
pthread_barrier_t barrier;

/*
* function prototypes
* computeDSV: used to compute new DSV value;
* updateDSV : used to update curDsv value to newDsv;
* dataInput : used to input data and populate the Box
* interSection : used to calculate the value of intersection of two Box
*/
void  computeDSV(int);
void  updateDSV(int);
void  dataInput(int*);

/*
* function: dataInput
************************
* read parameters from stdin and populate Boxes with Box structs
* terminate while id equals to -1
*/
void dataInput(int* numBox){


    int numR,numC,id;
    int j,x,y,w,h;

    scanf("%d %d %d", numBox,&numR,&numC);
    boxes = (struct Box *)malloc(sizeof(struct Box)*(*numBox));
    while(1){
        scanf("%d",&id);
        if(id==-1){
            break;
        }
        scanf( "%d %d %d %d",&y,&x,&h,&w);
        boxes[id].l=x;
        boxes[id].r=x+w;
        boxes[id].t=y;
        boxes[id].b=y+h;
        boxes[id].p=(h+w)*2;

        scanf( "%d",&boxes[id].nTop);
        if(boxes[id].nTop>0)
            boxes[id].topN = (int *)malloc(sizeof(int)*boxes[id].nTop);
        for(j=0;j<boxes[id].nTop;j++){
            scanf( "%d",&(boxes[id].topN[j]));
        }

        scanf( "%d",&boxes[id].nBottom);
        if(boxes[id].nBottom>0)
            boxes[id].bottomN = (int *)malloc(sizeof(int)*boxes[id].nBottom);
        for(j=0;j<boxes[id].nBottom;j++){
            scanf( "%d",&(boxes[id].bottomN[j]));
        }


        scanf( "%d",&boxes[id].nLeft);
        if(boxes[id].nLeft>0)
            boxes[id].leftN = (int *)malloc(sizeof(int)*boxes[id].nLeft);
        for(j=0;j<boxes[id].nLeft;j++){
            scanf( "%d",&(boxes[id].leftN[j]));
        }

        scanf( "%d",&boxes[id].nRight);
        if(boxes[id].nRight>0)
            boxes[id].rightN = (int *)malloc(sizeof(int)*boxes[id].nRight);
        for(j=0;j<boxes[id].nRight;j++){
            scanf( "%d",&(boxes[id].rightN[j]));
        }

        scanf( "%f",&(boxes[id].curDsv));
    }

}

/*
* function: converge
***********************
* use while loop to compute new dsv and  update dsv value
* if meets constrains -> max-min<max*epsilon -> break while loop
*
* args: numBox: the total number of boxes
* args: affect_rate, epsilon: read from stdin
*
* updates: the value of DSV for each box and the min and  max value of DSV
*/
void* converge(void* id){
    int   i=0;
    float min,max;
    long j=(long)id;
    int s=0;

    while (isConverge){




        for(i=j;i<numBox;i+=numThread){
        computeDSV(i);
        }

       pthread_barrier_wait(&barrier);



        for(i=j;i<numBox;i+=numThread){
            updateDSV(i);
        }


        s=pthread_barrier_wait(&barrier);


        if(s==PTHREAD_BARRIER_SERIAL_THREAD) {

            min=FLT_MAX;
            max=FLT_MIN;

            for(i=0;i<numBox;i++) {
                min=boxes[i].curDsv>min?min:boxes[i].curDsv;
                max=boxes[i].curDsv>max?boxes[i].curDsv:max;
            }
            count++;
            if((max-min)<(max*epsilon)) {
                printf("Convergence! min=%f,max=%f\n",min,max);
                printf("count= %d\n",count);
                isConverge=0;
            }
        }
        pthread_barrier_wait(&barrier);


    }
    pthread_exit(NULL);
}
/*
* function:  intersection
***************************
* used to compute two edges' intersection
*
* args: the start point and end point of each edges
* return: their intersection
*/
int intersection(int e1,int e2,int e3,int e4){
    int result=0;
    result=e4>e2?e2:e4;
    result-=e3>e1?e3:e1;
    return result;
}

/*
* function: computeDSV
***********************
* compute each side's adjacent temperature, sum them and divided by perimeter
*
* args: the id of box and affect_rate
* update: box's newDSV
*/
void computeDSV(int id) {
        float adjTemp,offsetDsv;
        adjTemp=0;
        int i=0,tempId=0;
        if (boxes[id].nLeft == 0 && boxes[id].curDsv != 0) {
            adjTemp += boxes[id].curDsv * (boxes[id].b - boxes[id].t);
        }
        for (i = 0; i < boxes[id].nLeft; i++) {
            tempId = boxes[id].leftN[i];
            if (boxes[tempId].curDsv != 0)
                adjTemp += boxes[tempId].curDsv * intersection(boxes[id].t, boxes[id].b, boxes[tempId].t, boxes[tempId].b);
        }

        if (boxes[id].nRight == 0) {
            adjTemp += boxes[id].curDsv * (boxes[id].b - boxes[id].t);
        }
        for (i = 0; i < boxes[id].nRight; i++) {
            tempId = boxes[id].rightN[i];
            if (boxes[tempId].curDsv != 0)
                adjTemp += boxes[tempId].curDsv * intersection(boxes[id].t, boxes[id].b, boxes[tempId].t, boxes[tempId].b);
        }

        if (boxes[id].nTop == 0) {
            if (boxes[tempId].curDsv != 0)
                adjTemp += boxes[id].curDsv * (boxes[id].r - boxes[id].l);
        }
        for (i = 0; i < boxes[id].nTop; i++) {
            tempId = boxes[id].topN[i];
            if (boxes[tempId].curDsv != 0)
                adjTemp += boxes[tempId].curDsv * intersection(boxes[id].l, boxes[id].r, boxes[tempId].l, boxes[tempId].r);
        }
        if (boxes[id].nBottom == 0) {
            if (boxes[tempId].curDsv != 0)
                adjTemp += boxes[id].curDsv * (boxes[id].r - boxes[id].l);
        }
        for (i = 0; i < boxes[id].nBottom; i++) {
            tempId = boxes[id].bottomN[i];
            if (boxes[tempId].curDsv != 0)
                adjTemp += boxes[tempId].curDsv * intersection(boxes[id].l, boxes[id].r, boxes[tempId].l, boxes[tempId].r);
        }


        adjTemp /= boxes[id].p;
        offsetDsv = (boxes[id].curDsv - adjTemp) * affect_rate;
        boxes[id].newDsv = (boxes[id].curDsv - offsetDsv);
}

/*
* function: updateDSV
* *********************
* copy newDsv value to curDsv
*
* args: box's id
* update: box's curDsv
*/
void updateDSV(int id) {
    boxes[id].curDsv=boxes[id].newDsv;
}

/*
* function: freeBoxes
*********************
* free all malloc memory
*/
void freeBoxes(int numBox){
    int i;
    for(i=0;i<numBox;i++){
        free(boxes[i].topN);
        free(boxes[i].bottomN);
        free(boxes[i].leftN);
        free(boxes[i].rightN);

    }
}

/*
************MAIN FUNCTION************
* inpute data -> coverage -> free box
*/
int main(int argc, char *argv[])
{
    //freopen("test_cases/testgrid_400_12206","r",stdin);
    /* code */
   // char     *file;
    int i=0;
    int s=0;
    clock_t 	c;
    time_t 	timerS, timerE;
    pthread_t *pthreadT;


    affect_rate = strtof(argv[1], NULL);
    epsilon = strtof(argv[2], NULL);
    numThread =atoi(argv[3]);

    dataInput(&numBox);
    c=clock();
    time(&timerS);

    pthreadT=(pthread_t *)malloc(sizeof(pthread_t)*numThread);


    s = pthread_barrier_init(&barrier, NULL, numThread);
    if (s != 0)
        printf("pthread_barrier_init fail, status=%d\n",s);


    for(i=0;i<numThread;i++){
        s=pthread_create(&pthreadT[i],NULL,converge,(void *)i);
        if (s != 0)
            printf("pthread_create fail, status=%d\n",s);
    }
    if(isConverge) {
        for (i = 0; i < numThread; i++) {
            s = pthread_join(pthreadT[i], NULL);
            if (s != 0)
                printf("pthread join fail, status=%d\n",s);
        }
    }
    pthread_barrier_destroy(&barrier);
    free(pthreadT);


    c=clock()-c;
    time(&timerE);

    printf ("It took %d clicks (%f seconds).\n",(int)c,((float)c)/CLOCKS_PER_SEC);
    printf("runtime is %f second\n",difftime(timerE,timerS));
    freeBoxes(numBox);
    free(boxes);

    return 0;
}

