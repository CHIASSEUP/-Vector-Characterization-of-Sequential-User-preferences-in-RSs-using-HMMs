#include "hmm.h"

#define MaxMovies 10
#define MaxUsers 10
#define MaxGenres 10
#define MaxClustersGenres 5
#define MaxNeighbours 5
#define MaxLanguages 20

// A movie
typedef struct Movie{      
	int Movie_Id;                    	 
	int Release_date;  
	int Run_time;
	int Original_language;
	float Vote_average;
	int Vote_count;
	int Genres[MaxGenres];
	int NbGenres;
	char Title[100];
} Movie;


// A movie watched by a user
typedef struct WatchedMovie{      
	int Movie_Id;                    	 
	float Score;                         
} WatchedMovie;

// A movie recommended to a user
typedef struct MovieRecom{      
	int Movie_Id; 
	int Frequency;
	double Distance;
	float Importance; 
} MovieRecom;

// A user
typedef struct User{      
	int User_Id;  
	int NbWatchedMovies;	
	WatchedMovie LastWatchedMovies[MaxWatchedMovies];
} User;

// An observation of a Markov chain
typedef struct Couple{      
	int State;                    	 
	int Symbol;                         
} Couple;

// A Markov chain
typedef struct MarkovChain{      
	int NbCouples;                
	Couple Elements[MaxWatchedMovies];        
} MC;

// Displays the content of a Markov chain
void DisplayMC(MC Delta){
	int i;
	
	for(i=0;(i<Delta.NbCouples);i++){
		printf("(%i,%i)\t",Delta.Elements[i].State,Delta.Elements[i].Symbol);
	}
	printf("\n");
	return;
}

// Prints the content of a Markov chain in a file
void DisplayMCfile(MC Delta, FILE ** f){
	int i;
	
	for(i=0;(i<Delta.NbCouples);i++){
		fprintf((*f),"(%i,%i)",Delta.Elements[i].State,Delta.Elements[i].Symbol);
	}
	fprintf((*f),"\n");
	return;
}

// Reads the movie data 
int ReadMovies(char * MoviesFileName, Movie TheMovies[MaxMovies]){
	FILE * f;
	float r;
	int i,j,k,Genre;
	char c;
	
	f = fopen(MoviesFileName, "rt");
	if(!f){
		printf("%s\n",MoviesFileName);
		return -1;
	} 
	i = 0;

	while(!feof(f)){
		fscanf(f,"%i,%i,%g,",&(TheMovies[i].Movie_Id),&(TheMovies[i].Release_date),&r);
		TheMovies[i].Run_time =  (int)r;
		fscanf(f,"%i,%g,%i,{",&(TheMovies[i].Original_language),&(TheMovies[i].Vote_average),&(TheMovies[i].Vote_count));
		j = 0;
		do{
			fscanf(f,"%i%c",&Genre,&c);
			TheMovies[i].Genres[j] = Genre;
			j++;
			if((c != ',')&&(c != '}')){
				printf("Movie genre format [line %i] [movie_id %i]\n",i+1,TheMovies[i].Movie_Id);
				return -1;
			}
		}while(c != '}');
		TheMovies[i].NbGenres = j;
		
		fscanf(f,",#");
		k = 0;
		do{
			fscanf(f,"%c",&c);
			if(c != '#'){
				TheMovies[i].Title[k] = c;
				k++;
			}
		}while(c != '#');
		TheMovies[i].Title[k] = 0;
		fscanf(f,"\n");
		i++;
	}

	fclose(f);
	return i;
}

// Reads the user data 
int ReadUsers(char * UsersFileName, User TheUsers[MaxUsers], int Tmax){
	FILE * f;
	int i,j,k;
	char c;
	WatchedMovie LastWatchedMovies[5000];
	
	f = fopen(UsersFileName, "rt");
	if(!f){
		printf("%s\n",UsersFileName);
		return -1;
	} 
	i = 0;
	
	while(!feof(f)){
		fscanf(f,"%i",&(TheUsers[i].User_Id));
		if(!feof(f)){
			j = 0;
			do{
				fscanf(f,"%c",&c);
				if(c == '('){
					fscanf(f,"%i,%f)",&(LastWatchedMovies[j].Movie_Id),&(LastWatchedMovies[j].Score));
					j++;
				}else if(c != '\n'){
					printf("User format\n");
					return -1;
				}
			}while(c != '\n');
			if(j <= Tmax){
				TheUsers[i].NbWatchedMovies = j;
				for(k=0;(k < j); k++){
					TheUsers[i].LastWatchedMovies[k].Movie_Id = LastWatchedMovies[k].Movie_Id;
					TheUsers[i].LastWatchedMovies[k].Score = LastWatchedMovies[k].Score;
				}
			}else{
				TheUsers[i].NbWatchedMovies = Tmax;
				for(k=0;(k < Tmax); k++){
					TheUsers[i].LastWatchedMovies[k].Movie_Id = LastWatchedMovies[(j-Tmax-1)+k].Movie_Id;
					TheUsers[i].LastWatchedMovies[k].Score = LastWatchedMovies[(j-Tmax-1)+k].Score;
				}
			}
			i++;
		}
	}
	fclose(f);
	return i;
}

// Sampling of the Score to obtain the corresponding state
int StateOf(float Score){
	int state;
	
	if((0.0 <= Score)&&(Score <= 1.0))state = 1;
	else if((1.0 < Score)&&(Score <= 2.0))state = 2; 
	else if((2.0 < Score)&&(Score <= 3.0))state = 3; 
	else if((3.0 < Score)&&(Score <= 4.0))state = 4; 
	else if(4.0 < Score)state = 5; 
	
	return state;
}

// Sampling of the Release_date to obtain the corresponding symbol
int SymbolOf_Release_date(int Release_date){
	int symbol;
	
	if(Release_date <= 1979)symbol = 1;
	else if((1980 <= Release_date)&&(Release_date <= 1985))symbol = 2; 
	else if((1986 <= Release_date)&&(Release_date <= 1990))symbol = 3; 
	else if((1991 <= Release_date)&&(Release_date <= 1995))symbol = 4; 
	else if((1996 <= Release_date)&&(Release_date <= 2000))symbol = 5; 
	else if((2001 <= Release_date)&&(Release_date <= 2005))symbol = 6; 
	else if((2006 <= Release_date)&&(Release_date <= 2010))symbol = 7; 
	else if((2011 <= Release_date)&&(Release_date <= 2015))symbol = 8; 
	else if((2016 <= Release_date)&&(Release_date <= 2020))symbol = 9; 
	else if(2021 <= Release_date)symbol = 10; 
	
	return symbol;
}


// Sampling of the Run_time to obtain the corresponding symbol
int SymbolOf_Run_time(int Run_time){
	int symbol;
	
	if(Run_time <= 60)symbol = 1;
	else if((61 <= Run_time)&&(Run_time <= 80))symbol = 2; 
	else if((81 <= Run_time)&&(Run_time <= 100))symbol = 3; 
	else if((101 <= Run_time)&&(Run_time <= 120))symbol = 4; 
	else if((121 <= Run_time)&&(Run_time <= 140))symbol = 5; 
	else if((141 <= Run_time)&&(Run_time <= 160))symbol = 6; 
	else if((161 <= Run_time)&&(Run_time <= 180))symbol = 7; 
	else if((181 <= Run_time)&&(Run_time <= 200))symbol = 8; 
	else if((201 <= Run_time)&&(Run_time <= 220))symbol = 9; 
	else if(221 <= Run_time)symbol = 10; 
	
	return symbol;
}


// Clustering of the Original_language to obtain the corresponding symbol
int SymbolOf_Original_language(int Original_language, int Frequency[MaxLanguages]){
	int symbol,k;
	
	k = Frequency[Original_language];
	
	if(500 <= k)symbol = 1;
	else if((200 <= k)&&(k < 500))symbol = 2; 
	else if((100 <= k)&&(k < 200))symbol = 3; 
	else if((40 <= k)&&(k < 100))symbol = 4; 
	else if((10 <= k)&&(k < 40))symbol = 5; 
	else if(k < 10)symbol = 6; 
	
	return symbol;
}

// Sampling of the Vote_average to obtain the corresponding symbol
int SymbolOf_Vote_average(float Vote_average){
	int symbol;
	
	if((0.0 <= Vote_average)&&(Vote_average <= 0.5))symbol = 1;
	else if((0.5 < Vote_average)&&(Vote_average <= 1.0))symbol = 2; 
	else if((1.0 < Vote_average)&&(Vote_average <= 1.5))symbol = 3; 
	else if((1.5 < Vote_average)&&(Vote_average <= 2.0))symbol = 4; 
	else if((2.0 < Vote_average)&&(Vote_average <= 2.5))symbol = 5; 
	else if((2.5 < Vote_average)&&(Vote_average <= 3.0))symbol = 6; 
	else if((3.0 < Vote_average)&&(Vote_average <= 3.5))symbol = 7; 
	else if((3.5 < Vote_average)&&(Vote_average <= 4.0))symbol = 8; 
	else if((4.0 < Vote_average)&&(Vote_average <= 4.5))symbol = 9; 
	else if(4.5 < Vote_average)symbol = 10; 
	
	return symbol;
}


// Sampling of the Vote_count to obtain the corresponding symbol
int SymbolOf_Vote_count(int Vote_count){
	int symbol;
	
	if((0 <= Vote_count)&&(Vote_count <= 25))symbol = 1;
	else if((26 <= Vote_count)&&(Vote_count <= 50))symbol = 2; 
	else if((51 <= Vote_count)&&(Vote_count <= 75))symbol = 3; 
	else if((76 <= Vote_count)&&(Vote_count <= 100))symbol = 4; 
	else if((101 <= Vote_count)&&(Vote_count <= 125))symbol = 5; 
	else if((126 <= Vote_count)&&(Vote_count <= 150))symbol = 6; 
	else if((151 <= Vote_count)&&(Vote_count <= 200))symbol = 7; 
	else if((201 <= Vote_count)&&(Vote_count <= 250))symbol = 8; 
	else if((251 <= Vote_count)&&(Vote_count <= 300))symbol = 9; 
	else if(301 <= Vote_count)symbol = 10; 
	
	return symbol;
}

// Sampling and clustering of the Genres to obtain the corresponding symbol
int SymbolOf_Genres(int Genres[MaxGenres], int NbGenres, int Frequency[MaxGenres], int NbClusters){
	int Binary[MaxClustersGenres], i,j,symbol,Cluster[MaxGenres];
	
	for(i=0;(i < NbGenres);i++){
		if(700 <= Frequency[Genres[i]]) Cluster[i] = 1;
		else if((400 <= Frequency[Genres[i]])&&(Frequency[Genres[i]] < 700)) Cluster[i] = 2;
		else if((100 <= Frequency[Genres[i]])&&(Frequency[Genres[i]] < 400)) Cluster[i] = 3;
		else if(Frequency[Genres[i]] < 100) Cluster[i] = 4;
	}

	for(j=0;(j < NbClusters);j++){
		Binary[j] = 0;
	}
	
	for(i=0;(i < NbGenres);i++){
		Binary[Cluster[i]-1] = 1;
	}

	symbol = Binary[0];
	for(j=1;(j < NbClusters);j++){
		symbol = 2*symbol + Binary[j];
	}
	
	return symbol;
}

// Collects the genre names and frequencies from a file
int GetFrequencyOfGenres(char * GenresFileName, int Frequency[MaxGenres], char GenreNames[MaxGenres][20]){
	FILE * f;
	int i,genre,Freq,NbGenres;
	char c;
	
	f = fopen(GenresFileName, "rt");
	if(!f){
		printf("%s\n",GenresFileName);
		return -1;
	} 

	NbGenres = 0;
	while(!feof(f)){
		i = 0;
		do{
			fread(&c,sizeof(char),1,f);
			if(c != ','){
				GenreNames[NbGenres][i] = c;
				i++;
			}else{
				GenreNames[NbGenres][i] = 0;
			}
		}while(c != ',');
		fscanf(f,"%i,%i\n",&genre,&Freq);
		Frequency[genre] = Freq;
		NbGenres++;
	}
	fclose(f);
	return NbGenres;
}

// Collects the language names and frequencies from a file
int GetFrequencyOfLanguages(char * LanguagesFileName, int Frequency[MaxLanguages], char LanguageNames[MaxLanguages][5]){
	FILE * f;
	int i,language,Freq,NbLanguages;
	char c;
	
	f = fopen(LanguagesFileName, "rt");
	if(!f){
		printf("%s\n",LanguagesFileName);
		return -1;
	} 

	NbLanguages = 0;
	while(!feof(f)){
		i = 0;
		do{
			fread(&c,sizeof(char),1,f);
			if(c != ','){
				LanguageNames[NbLanguages][i] = c;
				i++;
			}else{
				LanguageNames[NbLanguages][i] = 0;
			}
		}while(c != ',');
		fscanf(f,"%i,%i\n",&language,&Freq);
		Frequency[language] = Freq;
		NbLanguages++;
	}
	fclose(f);
	return NbLanguages;
}

// Index of a movie in the array 'TheMovies' according to its 'Movie_Id'
int IndexOfMovie(int Movie_Id, Movie TheMovies[MaxMovies], int NbMovies){
	int i;
	
	for(i=0;(i < NbMovies);i++){
		if(TheMovies[i].Movie_Id == Movie_Id){
			return i;
		}
	}
	return -1;
}

// Constructs the Markov chain associated with all the users
void ConstructMCsOfUser(User u, MC TheMCs[MaxFeatures], int SequencesOfUser[MaxFeatures][MaxWatchedMovies], int NbFeatures, Movie TheMovies[MaxMovies], int NbMovies, int FrequencyOfGenres[MaxGenres], int NbClustersGenres, int FrequencyOfLanguages[MaxLanguages], FILE ** f){
	int i, j, MovieIndex, state;
	
	for(j=0;(j < NbFeatures);j++){
		TheMCs[j].NbCouples = u.NbWatchedMovies;
	}

	for(i=0;(i < u.NbWatchedMovies);i++){
		/* States */
		state = StateOf(u.LastWatchedMovies[i].Score);
		for(j=0;(j < NbFeatures);j++){
			TheMCs[j].Elements[i].State = state;
		}
		
		/* Symbols */
		MovieIndex = IndexOfMovie(u.LastWatchedMovies[i].Movie_Id,TheMovies,NbMovies);
		if(MovieIndex != -1){
			TheMCs[0].Elements[i].Symbol = SymbolOf_Release_date(TheMovies[MovieIndex].Release_date);
			TheMCs[1].Elements[i].Symbol = SymbolOf_Run_time(TheMovies[MovieIndex].Run_time);
			TheMCs[2].Elements[i].Symbol = SymbolOf_Original_language(TheMovies[MovieIndex].Original_language,FrequencyOfLanguages);
			TheMCs[3].Elements[i].Symbol = SymbolOf_Vote_average(TheMovies[MovieIndex].Vote_average);
			TheMCs[4].Elements[i].Symbol = SymbolOf_Vote_count(TheMovies[MovieIndex].Vote_count);
			TheMCs[5].Elements[i].Symbol = SymbolOf_Genres(TheMovies[MovieIndex].Genres,TheMovies[MovieIndex].NbGenres,FrequencyOfGenres,NbClustersGenres);
			
			for(j=0;(j < NbFeatures);j++){
				SequencesOfUser[j][i] = TheMCs[j].Elements[i].Symbol - 1;
			}
		}
	}

	fprintf(*f,"_______________________________________________________________\n");
	for(j=0;(j < NbFeatures);j++){
		fprintf(*f,"delta(u_%i,%i):\t",u.User_Id,j+1);
		DisplayMCfile(TheMCs[j],f);
	}
	
	return;
}

// Constructs the initial HMMs associated with all the users
int InitialHMMsOfUser(MC TheMCsOfUser[MaxFeatures], int NbFeatures, HMM Lambda[MaxFeatures]){
	int NbSymbols, i,j,k,IndexFeature,State,NextState,Symbol,From[MaxStates],UseOfState[MaxStates];
	double Sum;
	
	NbSymbols = 0;
	
	for(IndexFeature=0;(IndexFeature < NbFeatures);IndexFeature++){
		
		Lambda[IndexFeature].N = 5; 
		
		if(IndexFeature == 2){
			/* Original language */
			Lambda[IndexFeature].M = 6;
		}else if(IndexFeature == 5){
			/* Genres */
			Lambda[IndexFeature].M = 15;  /* 2^(NbClusters) - 1 */
		}else{
			Lambda[IndexFeature].M = 10;
		}
		NbSymbols += Lambda[IndexFeature].M;
	}
	
	for(IndexFeature=0;(IndexFeature < NbFeatures);IndexFeature++){
		for(i=0;(i < Lambda[IndexFeature].N);i++){
			Lambda[IndexFeature].Pi[i] = 0.0;
			From[i] = 0;
			UseOfState[i] = 0;
			for(j=0;(j < Lambda[IndexFeature].N);j++){
				Lambda[IndexFeature].A[i][j] = 0.0;
			}
			for(j=0;(j < Lambda[IndexFeature].M);j++){
				Lambda[IndexFeature].B[i][j] = 0.0;
			}
		}

		for(j=0;(j < TheMCsOfUser[IndexFeature].NbCouples);j++){
			if(j != TheMCsOfUser[IndexFeature].NbCouples -1){
				State = TheMCsOfUser[IndexFeature].Elements[j].State - 1;
				NextState = TheMCsOfUser[IndexFeature].Elements[j+1].State - 1;
				Symbol = TheMCsOfUser[IndexFeature].Elements[j].Symbol - 1;

				From[State] += 1;
				UseOfState[State] += 1;
				Lambda[IndexFeature].A[State][NextState] += 1.0;
				Lambda[IndexFeature].B[State][Symbol] += 1.0;
				
				if(j == 0){
					Lambda[IndexFeature].Pi[State] += 1.0;
				}
			}else{
				State = TheMCsOfUser[IndexFeature].Elements[j].State - 1;
				Symbol = TheMCsOfUser[IndexFeature].Elements[j].Symbol - 1;

				UseOfState[State] += 1;
				Lambda[IndexFeature].B[State][Symbol] += 1.0;
			}
			
		}	
	
		for(i=0;(i < Lambda[IndexFeature].N);i++){
			Lambda[IndexFeature].Pi[i] /= (1+Epsi);
			for(j=0;(j < Lambda[IndexFeature].N);j++){
				Lambda[IndexFeature].A[i][j] /= (From[i]+Epsi);
			}
		}
		for(j=0;(j < Lambda[IndexFeature].N);j++){
			for(k=0;(k < Lambda[IndexFeature].M);k++){
				Lambda[IndexFeature].B[j][k] /= (UseOfState[j]+Epsi);
			}
		}

		/* Readjustment of Pi */
		Sum = 0.0;
		for(i=0;(i < Lambda[IndexFeature].N);i++){
			Sum += Lambda[IndexFeature].Pi[i];
		}	
		Sum = fabs(1.0 - Sum)/Lambda[IndexFeature].N;
		for(i=0;(i < Lambda[IndexFeature].N);i++){
			Lambda[IndexFeature].Pi[i] += Sum;
		}	
		
		/* Readjustment of A */
		for(i=0;(i < Lambda[IndexFeature].N);i++){
			Sum = 0.0;
			for(j=0;(j < Lambda[IndexFeature].N);j++){
				Sum += Lambda[IndexFeature].A[i][j];
			}
			Sum = fabs(1.0 - Sum)/Lambda[IndexFeature].N;
			for(j=0;(j < Lambda[IndexFeature].N);j++){
				Lambda[IndexFeature].A[i][j] += Sum;
			}	
		}	
		
		/* Readjustment of B */
		for(j=0;(j < Lambda[IndexFeature].N);j++){
			Sum = 0.0;
			for(k=0;(k < Lambda[IndexFeature].M);k++){
				Sum += Lambda[IndexFeature].B[j][k];
			}
			Sum = fabs(1.0 - Sum)/Lambda[IndexFeature].M;
			for(k=0;(k < Lambda[IndexFeature].M);k++){
				Lambda[IndexFeature].B[j][k] += Sum;
			}	
		}
		
		/* Not necessary at this level */
		StationaryDistribution(&Lambda[IndexFeature]);
	}
	
	return NbSymbols;
}	

// Swaps two doubles
void swapDouble(double* a, double* b) {
    double temp = *a;
    *a = *b;
    *b = temp;
}

// Swaps two floats
void swapFloat(float* a, float* b) {
    float temp = *a;
    *a = *b;
    *b = temp;
}

// Swaps two integers
void swapInt(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

// Function of partitionnement for the QuickSortKNN
int PartitionKNN(double distances[MaxUsers], int indices[MaxUsers], int low, int high) {
    double pivot = distances[high]; // pivot
    int i = (low - 1); // Index of the smallest element

    for (int j = low; j <= high - 1; j++) {
        // If the current element is lower than pivot
        if (distances[j] < pivot) {
            i++; // Increments the index of the smallest element
            swapDouble(&distances[i], &distances[j]);
            swapInt(&indices[i], &indices[j]);
        }
    }
    swapDouble(&distances[i + 1], &distances[high]);
    swapInt(&indices[i + 1], &indices[high]);
    return (i + 1);
}

// QuickSort KNN
void QuicksortKNN(double distances[MaxUsers], int indices[MaxUsers], int low, int high) {
    if (low < high) {
        int pi = PartitionKNN(distances, indices, low, high);
        QuicksortKNN(distances, indices, low, pi - 1);
        QuicksortKNN(distances, indices, pi + 1, high);
    }
}



// KNN algorithm 
void KNN(int NbNeighbours, int NbVectors, int NbComponents, int Dist, double FinalVect[MaxUsers][MaxFeatures*MaxSymbols], int TheNeighbours[MaxUsers][MaxNeighbours], double NeighbourDistances[MaxUsers][MaxNeighbours], double distances[MaxUsers][MaxUsers]) {
    int i,j,m, indices[MaxUsers];

    for (i = 0; i < NbVectors; i++) {

        // computes the distance between the i-th vector and all the system vectors
        for (j = 0; j < NbVectors; j++) {
            if (i != j) {
				if(Dist == 0){
					distances[i][j] = DistanceEuclidienne(FinalVect[i], FinalVect[j], NbComponents);
				}else{
					distances[i][j] = DistanceManhattan(FinalVect[i], FinalVect[j], NbComponents);
				}
            } else {
				// An infinite distance between the i-th vector and itself
                distances[i][j] = DBL_MAX; // 
            }
            indices[j] = j;
        }

        // QuickSort of the distances and the indices
		QuicksortKNN(distances[i],indices,0,NbVectors-1);
        // Records the K nearest neighbors and the corresponding distances
        for (m = 0; m < NbNeighbours; m++) {
            TheNeighbours[i][m] = indices[m];
			NeighbourDistances[i][m] = distances[i][m];
        }
    }
	return;
}

// Index of a recommended movie in the array 'TheMovieRecom' according to its 'Movie_Id'
int IndexOfMovieRecom(int Movie_Id, MovieRecom TheMovieRecom[MaxNeighbours*MaxWatchedMovies], int NbMovieRecom){
	int k;
	
	for (k = 0; k < NbMovieRecom; k++) {
		if(TheMovieRecom[k].Movie_Id == Movie_Id){
			return k;
		}
	}
	return -1;
}

// Computes the initial set of recommended movies (Equation 8 in the paper)
// and their corresponding Importance  (Equation 9 in the paper)
void InitMovieRecom(MovieRecom TheMovieRecom[MaxUsers][MaxNeighbours*MaxWatchedMovies], int NbMovieRecom[MaxUsers], int TheNeighbours[MaxUsers][MaxNeighbours], double NeighbourDistances[MaxUsers][MaxNeighbours],  int NbNeighbours, User TheUsers[MaxUsers], int NbUsers){
	int i,j,k,l,m,n;
	double epsi = 1e-20;
	
	for (i = 0; i < NbUsers; i++) {
		NbMovieRecom[i] = 0;
		for(k=0;(k < MaxNeighbours*MaxWatchedMovies);k++){
			TheMovieRecom[i][k].Frequency = -1;
		}
	}	

	for (i = 0; i < NbUsers; i++) {
		for(k=0;(k < NbNeighbours);k++){
			/* l is the index of the current (k-th) neighbour of the i-th user */
			l = TheNeighbours[i][k];
			
			/* Browsing the movies watched by the current neighbour i-th user */
			for(j=0;(j < TheUsers[l].NbWatchedMovies);j++){
				m = TheUsers[l].LastWatchedMovies[j].Movie_Id;
				n = IndexOfMovieRecom(m,TheMovieRecom[i],NbMovieRecom[i]);
				if(n == -1){
					/* Inserting a new movie in the set of recommendations */
					TheMovieRecom[i][NbMovieRecom[i]].Movie_Id = m;
					TheMovieRecom[i][NbMovieRecom[i]].Frequency = 1;
					TheMovieRecom[i][NbMovieRecom[i]].Distance = NeighbourDistances[i][k];
					TheMovieRecom[i][NbMovieRecom[i]].Importance = (TheUsers[l].LastWatchedMovies[j].Score/(NeighbourDistances[i][k]+epsi));
					NbMovieRecom[i]++;
				}else{
					/* Updating an existing movie in the set of recommendations */
					TheMovieRecom[i][n].Frequency += 1;
					TheMovieRecom[i][n].Importance += (TheUsers[l].LastWatchedMovies[j].Score/(NeighbourDistances[i][k]+epsi));
				}
			}
		}
		/* Updating the average score of all the movies in the set of recommendations */
		for(j=0;(j < NbMovieRecom[i]);j++){
			TheMovieRecom[i][j].Importance *= TheMovieRecom[i][j].Frequency;
		}
	}
	return;
}


// Function of partitionnement for the QuickSortMovies
int PartitionMovies(MovieRecom TheMovieRecom[MaxNeighbours*MaxWatchedMovies], int low, int high) {
    float pivot = TheMovieRecom[high].Importance; // pivot
    int i = (low - 1); // Index of the highest element

    for (int j = low; j <= high - 1; j++) {
        // If the current element is greater than pivot
        if (TheMovieRecom[j].Importance > pivot) {
            i++; // Increments the index of the highest element 
			
            swapInt(&TheMovieRecom[i].Movie_Id, &TheMovieRecom[j].Movie_Id);
            swapInt(&TheMovieRecom[i].Frequency, &TheMovieRecom[j].Frequency);
            swapDouble(&TheMovieRecom[i].Distance, &TheMovieRecom[j].Distance);
            swapFloat(&TheMovieRecom[i].Importance, &TheMovieRecom[j].Importance);
        }
    }
    swapInt(&TheMovieRecom[i+1].Movie_Id, &TheMovieRecom[high].Movie_Id);
    swapInt(&TheMovieRecom[i+1].Frequency, &TheMovieRecom[high].Frequency);
    swapDouble(&TheMovieRecom[i+1].Distance, &TheMovieRecom[high].Distance);
    swapFloat(&TheMovieRecom[i+1].Importance, &TheMovieRecom[high].Importance);
    return (i + 1);
}


// QuickSort Movies
void QuicksortMovies(MovieRecom TheMovieRecom[MaxNeighbours*MaxWatchedMovies], int low, int high) {
    if (low < high) {
        int pi = PartitionMovies(TheMovieRecom, low, high);
        QuicksortMovies(TheMovieRecom, low, pi - 1);
        QuicksortMovies(TheMovieRecom, pi + 1, high);
    }
}

// Sort the recommended movies in decrease order of their importance 
void SortMovieRecom_by_Importance(MovieRecom TheMovieRecom[MaxUsers][MaxNeighbours*MaxWatchedMovies], int NbMovieRecom[MaxUsers], int NbUsers){
	int i;
	
	for (i = 0; i < NbUsers; i++) {
		QuicksortMovies(TheMovieRecom[i],0,NbMovieRecom[i]-1);
	}
	return;
}

