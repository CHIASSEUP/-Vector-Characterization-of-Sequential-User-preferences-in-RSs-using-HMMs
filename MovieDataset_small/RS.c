#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>
#include <ctype.h>
#include <time.h>
#include "RS.h"


/*
________________________________________________________________________________________________   
   Compilation command line:

   gcc -Wall -Wl,--stack,536870912 -o RS.exe RS.c
________________________________________________________________________________________________   
   Execution command line:

   RS.exe MoviesFile UsersFile MCsFile FrequencyOfGenresFile FrequencyOfLanguagesFile 
          NbFeatures MaxIterations NbNeighbours MaxRecoms Tmax SaveHMMs
________________________________________________________________________________________________   
   Example: 
   
   RS.exe movies.txt users.txt MCs.txt FrequencyOfGenres.txt FrequencyOfLanguages.txt 6 100 30 20 200 0
________________________________________________________________________________________________   
*/

int main(int NbArg, char ** MesArg) {
	Movie TheMovies[MaxMovies];
	User TheUsers[MaxUsers];
	MC TheMCsOfUser[MaxFeatures];
	MovieRecom TheMovieRecom[MaxUsers][MaxNeighbours*MaxWatchedMovies];
	HMM Lambda_Init[MaxFeatures],Lambda[MaxFeatures];
	double Vect[MaxFeatures][MaxSymbols], FinalVect[MaxUsers][MaxFeatures*MaxSymbols];
	double NeighbourDistances[2][MaxUsers][MaxNeighbours], F1[2],std_dev[2],epsilon = 1e-10;
	char GenreNames[MaxGenres][20], LanguageNames[MaxLanguages][5]; 

	int i,j,k,l,NbMovies, NbUsers, NbFeatures, NbGenres,NbClusters, FrequencyOfGenres[MaxGenres];
	int MaxIterations, SequencesOfUser[MaxFeatures][MaxWatchedMovies], NbMovieRecom[MaxUsers];
	int SaveHMMs, NbSymbols, NbNeighbours,Dist,TheNeighbours[2][MaxUsers][MaxNeighbours];
	int MaxRecoms, NbLanguages, Tmax, FrequencyOfLanguages[MaxLanguages];
	FILE *f, *g[MaxFeatures], *h, *w, *z;
	char HMM_Init_FileName[100],HMM_FileName[100],FeatureVector_FileName[100],FeatureIndex[5],UserIndex[5]; 

	clock_t overall_t;
	double OverallDuration;
	
	srand(time(NULL));

	NbClusters = 4;
	NbFeatures = atoi(MesArg[6]);
	MaxIterations = atoi(MesArg[7]);
	NbNeighbours = atoi(MesArg[8]);
	MaxRecoms = atoi(MesArg[9]); 
	Tmax = atoi(MesArg[10]);
	if(Tmax > MaxWatchedMovies){
		Tmax = MaxWatchedMovies;
	}
	SaveHMMs = atoi(MesArg[11]);
	
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
	
	w = fopen("Neighbours.txt", "wt");
	if(!w){
		printf("Neighbours file\n");
		return -1;
	} 
	
	z = fopen("Recommendations.txt", "wt");
	if(!z){
		printf("Recommendations file\n");
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
		if((i != 0)&&(i % 10 == 0)){
			printf(".");
		}
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
	printf("\nVector computation time = %g ms\n",OverallDuration);
		
	/* Calculating the indices of the (NbNeighbours) nearest neighbours of each user */
	
	for(Dist = 0; Dist <= 1; Dist++){
		if(Dist == 0){
			fprintf(w,"KNN with the Euclidean distance\n\n");
			fprintf(z,"KNN with the Euclidean distance\n\n");
		}else{
			fprintf(w,"KNN with the Manhattan distance\n\n");
			fprintf(z,"KNN with the Manhattan distance\n\n");
		}

		overall_t = clock(); 

		/* KNN algorithm */
		KNN(NbNeighbours,NbUsers,NbSymbols,Dist,FinalVect,TheNeighbours[Dist],NeighbourDistances[Dist]);

		/* Calculating the initial movie recommendations*/
		InitMovieRecom(TheMovieRecom,NbMovieRecom,TheNeighbours[Dist],NeighbourDistances[Dist],NbNeighbours,TheUsers,NbUsers);
		/* Sorting the movie recommendations by frequencies */
		SortMovieRecom_by_Importance(TheMovieRecom,NbMovieRecom,NbUsers);

		overall_t = clock() - overall_t;
		OverallDuration = ((double)overall_t)*1000.0/CLOCKS_PER_SEC;
		printf("\nRecommendation time[%i] = %g ms\n",Dist,OverallDuration);
		
		/* Saving the result of the KNN algorithm and the initial movie recommendations*/
		for (i = 0; i < NbUsers; i++) {
		
			fprintf(w,"%i,[",TheUsers[i].User_Id);
			fprintf(z,"%i,",TheUsers[i].User_Id);
			for(j=0;(j < NbNeighbours);j++){
				fprintf(w,"%i",TheUsers[TheNeighbours[Dist][i][j]].User_Id);
				if(j != NbNeighbours-1){
					fprintf(w,",");
				}else{
					fprintf(w,"]\t");
				}
			}	
		
			fprintf(w,"(");
			fprintf(z,"(");
			for(j=0;(j < TheUsers[i].NbWatchedMovies);j++){
				fprintf(w,"%i",TheUsers[i].LastWatchedMovies[j].Movie_Id);
				fprintf(z,"%i",TheUsers[i].LastWatchedMovies[j].Movie_Id);
				if(j != TheUsers[i].NbWatchedMovies-1){
					fprintf(w,",");
					fprintf(z,",");
				}else{
					fprintf(w,") --> {%i}[",NbMovieRecom[i]);
					fprintf(z,") --> [");
				}
			}

			for(k=0;(k < NbMovieRecom[i]);k++){
				fprintf(w,"(%i,",TheMovieRecom[i][k].Movie_Id);
				fprintf(w,"%.2f)",TheMovieRecom[i][k].Importance);
				
				if(k < MaxRecoms){
					fprintf(z,"(%i,",TheMovieRecom[i][k].Movie_Id);
					fprintf(z,"%.2f)",TheMovieRecom[i][k].Importance);
				}
			}
			fprintf(w,"]\n");
			fprintf(z,"]\n");
		}
		fprintf(w,"_______________________________________________________________\n");
		fprintf(z,"_______________________________________________________________\n");
		
		F1[Dist] = F1_measure(TheUsers,NbUsers,TheMovieRecom,MaxRecoms,&std_dev[Dist]);
		printf("F1[%i] = %.3f (+/-) %g\n",Dist,F1[Dist],std_dev[Dist]);
	}

	/* Closing all the result files */
	fclose(f);
	for(j=0;(j < NbFeatures);j++){
		fclose(g[j]);
	}	
	fclose(h);
	fclose(w);
	fclose(z);
	
	return 0;
}