//
// Created by Xia Li on 3/5/16.
//
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <time.h>
#include <pthread.h>

/*
* class definitions for Box
* l:left upper X ; r: right upper X ; t: left upper Y ; b: left down Y; p:parameter
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
float affect_rate;
int numBox,numThread;

/*
* function prototypes
* computeDSV: used to compute new DSV value;
* updateDSV : used to update curDsv value to newDsv;
* dataInput : used to input data and populate the Box
* interSection : used to calculate the value of intersection of two Box
*/
void*  computeDSV(void*);
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
* function: coverage
***********************
* use while loop to compute new dsv and  update dsv value
* if meets constrains -> max-min<max*epsilon -> break while loop
*
* args: numBox: the total number of boxes
* args: affect_rate, epsilon: read from stdin
*
* updates: the value of DSV for each box and the min and  max value of DSV
*/
void coverage(float epsilon){
    int   i,j,count=0;
    float min,max;

    int thindex = 0;


    while (1){
        count++;
        min= FLT_MAX;
        max= FLT_MIN;
        pthread_t *pthreadT;
        pthreadT=(pthread_t*)malloc(sizeof(pthread_t)*numThread);



        for(i=0;i<numThread;i++){
            pthread_create(&pthreadT[i],NULL,computeDSV,(void *)i);
        }
        for(j=0;j<numThread;j++){
            pthread_join(pthreadT[j],NULL);
        }


        free(pthreadT);

        for(i=0;i<numBox;i++) {
            min=boxes[i].newDsv>min?min:boxes[i].newDsv;
            max=boxes[i].newDsv>max?boxes[i].newDsv:max;
        }


        if((max-min)<(max*epsilon)) {
            printf("Convergence! min=%f,max=%f\n",min,max);
            printf("count= %d\n",count);
            break;
        }

        for(i=0;i<numBox;i++){
            updateDSV(i);
        }
    }
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
* args: the id of box
* update: box's newDSV
*/
void* computeDSV(void* Id) {
    int id;
    long j=(long)Id;
    for(id=j;id<numBox;id+=numThread) {
    float adjTemp=0,offsetDsv=0;
    adjTemp=0;
    int i, tempId = 0;



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
    pthread_exit(NULL);

}

/*
* function: updateDSV
* *********************
* copy newDsv value to curDsv
*s
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
* input data -> coverage -> free box
*/
int main(int argc, char *argv[])
{
    //freopen("test_cases/testgrid_400_12206","r",stdin);
    /* code */
    clock_t 	c;
    time_t 	timerS, timerE;
    float epsilon;

    affect_rate = (float)strtod(argv[1], NULL);
    epsilon = (float) strtod(argv[2], NULL);
    numThread=(int)strtod(argv[3],NULL);

    dataInput(&numBox);
    c=clock();
    time(&timerS);
    coverage(epsilon);
    c=clock()-c;
    time(&timerE);

    printf ("It took %d clicks (%f seconds).\n",(int)c,((float)c)/CLOCKS_PER_SEC);
    printf("runtime is %f second\n",difftime(timerE,timerS));
    freeBoxes(numBox);
    free(boxes);

    return 0;
}



