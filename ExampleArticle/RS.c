#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>
#include <time.h>
#include "RS.h"

/*
____________________________________________________________________________________________   
   Compilation command line:

   gcc -Wall -o RS.exe RS.c
________________________________________________________________________________________________   
   Execution command line:

   RS.exe MoviesFile UsersFile MCsFile FrequencyOfGenresFile FrequencyOfLanguagesFile 
          NbFeatures MaxIterations Tmax SaveHMMs
________________________________________________________________________________________________   
   Example: 
   
   RS.exe movies.txt users.txt MCs.txt FrequencyOfGenres.txt FrequencyOfLanguages.txt 6 100 10 0
________________________________________________________________________________________________   
*/

int main(int NbArg, char ** MesArg) {
	Movie TheMovies[MaxMovies];
	User TheUsers[MaxUsers];
	MC TheMCsOfUser[MaxFeatures];
	HMM Lambda_Init[MaxFeatures],Lambda[MaxFeatures];
	char GenreNames[MaxGenres][20], LanguageNames[MaxLanguages][5]; 
	double Vect[MaxFeatures][MaxSymbols], FinalVect[MaxUsers][MaxFeatures*MaxSymbols], epsilon = 1e-10;

	int i,j,k,l,NbMovies, NbUsers, NbFeatures, NbGenres,NbClusters, FrequencyOfGenres[MaxGenres];
	int MaxIterations, SequencesOfUser[MaxFeatures][MaxWatchedMovies];
	int SaveHMMs, NbSymbols;
	int NbLanguages, Tmax, FrequencyOfLanguages[MaxLanguages];
	FILE *f, *g[MaxFeatures], *h;
	char HMM_Init_FileName[100],HMM_FileName[100],FeatureVector_FileName[100],FeatureIndex[5],UserIndex[5]; 
	
	clock_t overall_t;
	double OverallDuration;
	
	srand(time(NULL));

	NbClusters = 4;
	NbFeatures = atoi(MesArg[6]);
	MaxIterations = atoi(MesArg[7]);
	Tmax = atoi(MesArg[8]);   
	if(Tmax > MaxWatchedMovies){
		Tmax = MaxWatchedMovies;
	}
	SaveHMMs = atoi(MesArg[9]);
	
	/* Opening all the result files */
	f = fopen(MesArg[3], "wt");
	if(!f){
		printf("MCs file\n");
		return -1;
	} 
	
	for(j=0;(j < NbFeatures);j++){
		
		itoa(j+1,FeatureIndex,10);
		strcpy(FeatureVector_FileName,"FeatureVector");
		strcat(FeatureVector_FileName,FeatureIndex);
		strcat(FeatureVector_FileName,".bin");
		
		g[j] = fopen(FeatureVector_FileName, "wt");
		if(!g[j]){
			printf("FeatureVectors%i file\n",j+1);
			return -1;
		} 
	}
	
	h = fopen("FinalVectors.txt", "wt");
	if(!h){
		printf("FinalVectors file\n");
		return -1;
	} 

	overall_t = clock(); 

	NbMovies = ReadMovies(MesArg[1],TheMovies);
	printf("%i Movies\n",NbMovies);
	
	NbUsers = ReadUsers(MesArg[2],TheUsers,Tmax);
	printf("%i Users\n",NbUsers);
	
	NbGenres = GetFrequencyOfGenres(MesArg[4],FrequencyOfGenres,GenreNames);
	printf("%i Genres\n",NbGenres);
	
	NbLanguages = GetFrequencyOfLanguages(MesArg[5],FrequencyOfLanguages,LanguageNames);
	printf("%i Languages\n",NbLanguages);
	
	
	for(i=0;(i < NbUsers);i++){
		
		fprintf(h,"%i,[",TheUsers[i].User_Id);
		
		ConstructMCsOfUser(TheUsers[i],TheMCsOfUser,SequencesOfUser,NbFeatures,TheMovies,NbMovies,FrequencyOfGenres,NbClusters,FrequencyOfLanguages,&f);
		
		/* Computing the initial HMMs */
		NbSymbols = InitialHMMsOfUser(TheMCsOfUser,NbFeatures,Lambda_Init);
		
		l = 0;
		for(j=0;(j < NbFeatures);j++){
			
			/* Training each HMM */
			Baum_Welch_Mono(Lambda_Init[j],SequencesOfUser[j],TheUsers[i].NbWatchedMovies,epsilon,MaxIterations,&(Lambda[j]));

			/* Computing each feature vector */
			VecteurHMM(Lambda[j],Vect[j]);

			fprintf(g[j],"%i,[",TheUsers[i].User_Id);
			for(k=0;(k < Lambda[j].M);k++){
				/* Saving the feature vector components */
				fprintf(g[j],"%g",Vect[j][k]);
				if(k != Lambda[j].M-1){
					fprintf(g[j],",");
				}else{
					fprintf(g[j],"]\n");
				}
				
				/* Saving the final vector components */
				FinalVect[i][l++] = Vect[j][k];
				fprintf(h,"%g",Vect[j][k]);
				if((j != NbFeatures-1)||(k != Lambda[j].M-1)){
					fprintf(h,",");
				}else{
					fprintf(h,"]\n");
				}
			}
			
			/* Saving the initial and the trained HMMs */
			if(SaveHMMs == 1){
				itoa(j+1,FeatureIndex,10);
				itoa(TheUsers[i].User_Id,UserIndex,10);
				
				strcpy(HMM_Init_FileName,"HMM_Init_user");
				strcat(HMM_Init_FileName,UserIndex);
				strcat(HMM_Init_FileName,"_feature");
				strcat(HMM_Init_FileName,FeatureIndex);
				strcat(HMM_Init_FileName,".dat");
				EnregistreHMMtxt(Lambda_Init[j],HMM_Init_FileName);
				
				strcpy(HMM_FileName,"HMM_user");
				strcat(HMM_FileName,UserIndex);
				strcat(HMM_FileName,"_feature");
				strcat(HMM_FileName,FeatureIndex);
				strcat(HMM_FileName,".dat");
				EnregistreHMMtxt(Lambda[j],HMM_FileName);
			}
		}
	}

	overall_t = clock() - overall_t;
	OverallDuration = ((double)overall_t)*1000.0/CLOCKS_PER_SEC;
	printf("Vector computation time = %g ms\n",OverallDuration);
	

	printf("Eucli(1,2) = %.2f\n",DistanceEuclidienne(FinalVect[0],FinalVect[1],NbSymbols));
	printf("Eucli(1,3) = %.2f\n",DistanceEuclidienne(FinalVect[0],FinalVect[2],NbSymbols));
	printf("Eucli(2,3) = %.2f\n",DistanceEuclidienne(FinalVect[1],FinalVect[2],NbSymbols));
	

	printf("Manha(1,2) = %.2f\n",DistanceManhattan(FinalVect[0],FinalVect[1],NbSymbols));
	printf("Manha(1,3) = %.2f\n",DistanceManhattan(FinalVect[0],FinalVect[2],NbSymbols));
	printf("Manha(2,3) = %.2f\n",DistanceManhattan(FinalVect[1],FinalVect[2],NbSymbols));

	/* Closing all the result files */
	fclose(f);
	for(j=0;(j < NbFeatures);j++){
		fclose(g[j]);
	}	
	fclose(h);
	
	return 0;
}