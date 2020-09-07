#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#define NB_WIN 5
#define SIZE 10

#define FULL					\
  int act = 0;					\
  if((argc==2)&&!strcmp(argv[1],"ON")){		\
    act=1;					\
  }

#define INIT_TEST(carac,act)			\
  long clk_tck = CLOCKS_PER_SEC;		\
  clock_t t1, t2;				\
  t1 = clock();					\
  if (act){					\
    for(int i=0;i<10;i++){			\
      printf("%s",carac);			\
    }						\
    printf("\n");				\
  }						\
  int res=0;int tot=0;int val;

#define TEST(name_test,test,act)		\
  tot++;					\
  val = test;					\
  if (val) res++;					\
  if (act){						\
    printf("Test %s : ",name_test);			\
    (!val?printf("FAILURE\n"):printf("SUCCESS\n"));	\
  }

#define END_TEST(carac,act)			\
  t2 = clock();					\
  if (act){					\
    for(int i=0;i<10;i++){			\
      printf("%s",carac);			\
    }						\
    printf("\n");				\
    (res==tot)?					\
      printf("SUCCESS : %d / %d \n",res,tot):	\
      printf("FAILURE : %d / %d \n",res,tot);	\
    for(int i=0;i<10;i++){			\
      printf("%s",carac);			\
    }								\
    printf("\nElapsed time : %lf \n",(double)(t2-t1)/(double)clk_tck);	\
    for(int i=0;i<10;i++){						\
      printf("%s",carac);						\
    }									\
    printf("\n");							\
  }

#endif
