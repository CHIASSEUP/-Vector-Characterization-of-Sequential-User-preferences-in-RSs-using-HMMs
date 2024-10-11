#define MaxWatchedMovies 10	        

#define MaxStates 5 			         /* Max number of states */
#define MaxSymbols 15           		 /* Max number of symbols */
#define MaxFeatures 6
#define MinDouble 1e-80                  
#define MinFloat 1e-7                   
#define Epsi 1.0                           


// A HMM
typedef struct HMM HMM;
struct HMM
{
	int N;
	int M;
	double A[MaxStates][MaxStates];
	double B[MaxStates][MaxSymbols];
	double Pi[MaxStates];
	double Phi[MaxStates];				
};

// Adjustment of a HMM
void AdjustHMM(HMM * Lambda){
	int i,j,k;
	double Som;
	
		/* Reajustement Pi */
		
		Som = 0.0;
		for(i=0;(i < (*Lambda).N);i++){
			Som += (*Lambda).Pi[i];
		}	
		Som = fabs(1.0 - Som)/(*Lambda).N;
		for(i=0;(i < (*Lambda).N);i++){
			(*Lambda).Pi[i] += Som;
		}	
		/* Reajustement A */	
		for(i=0;(i < (*Lambda).N);i++){
			Som = 0.0;
			for(j=0;(j < (*Lambda).N);j++){
				Som += (*Lambda).A[i][j];
			}
			Som = fabs(1.0 - Som)/(*Lambda).N;
			for(j=0;(j < (*Lambda).N);j++){
				(*Lambda).A[i][j] += Som;
			}	
		}	
		/* Reajustement b */	
		for(j=0;(j < (*Lambda).N);j++){
			Som = 0.0;
			for(k=0;(k < (*Lambda).M);k++){
				Som += (*Lambda).B[j][k];
			}
			Som = fabs(1.0 - Som)/(*Lambda).M;
			for(k=0;(k < (*Lambda).M);k++){
				(*Lambda).B[j][k] += Som;
			}	
		}

	return;
}

// Replaces low probability values by zeros in the HMM 
void CleanHMM(HMM * Lambda){
	int i,j;

	for(i=0;(i < (*Lambda).N);i++){
		for(j=0;(j < (*Lambda).N);j++){
			if((*Lambda).A[i][j] < MinFloat) (*Lambda).A[i][j] = 0.0;
		}
	}
	for(i=0;(i < (*Lambda).N);i++){
		for(j=0;(j < (*Lambda).M);j++){
			if((*Lambda).B[i][j] < MinFloat) (*Lambda).B[i][j] = 0.0;
		}
	}
	for(i=0;(i < (*Lambda).N);i++){
		if((*Lambda).Pi[i] < MinFloat) (*Lambda).Pi[i] = 0.0;
	}
	for(i=0;(i < (*Lambda).N);i++){
		if((*Lambda).Phi[i] < MinFloat) (*Lambda).Phi[i] = 0.0;
	}
	return;
}


// Copy of a HMM
void CopyHMM(HMM * Lambda1, HMM Lambda2){
	int i,j;
	
	(*Lambda1).N = Lambda2.N;
	(*Lambda1).M = Lambda2.M;
	for(i = 0; i < Lambda2.N; i++)
	{
		(*Lambda1).Pi[i] = Lambda2.Pi[i];	
		(*Lambda1).Phi[i] = Lambda2.Phi[i];	
		for(j = 0; j < Lambda2.N; j++)
		{
			(*Lambda1).A[i][j] = Lambda2.A[i][j];
		}
		for(j = 0; j < Lambda2.M; j++)
		{
			(*Lambda1).B[i][j] = Lambda2.B[i][j];
		}
	}
	return;
}

// Saves a HMM in a textfile
void EnregistreHMMtxt(HMM Lambda, char * FichierSortie){
	int i,j;
	FILE * fichier;
	
	fichier = fopen(FichierSortie, "wt");
	fprintf(fichier, "\nParameters: %i states AND %i symbols\n\n", Lambda.N, Lambda.M);
	fprintf(fichier, "Matrix A :\n\n");

	for(i = 0; i < Lambda.N; i++)
	{
		for(j = 0; j < Lambda.N; j++)
			fprintf(fichier, "%g\t", Lambda.A[i][j]);

		fprintf(fichier, "\n");
 	}

	fprintf(fichier, "\n\nMatrix B :\n\n");

	for(i = 0; i < Lambda.N; i++)
	{
		for(j = 0; j < Lambda.M; j++)
			fprintf(fichier, "%g\t", Lambda.B[i][j]);

		fprintf(fichier, "\n");
 	}

	fprintf(fichier, "\n\nVector Pi :\n\n");

	for(j = 0; j < Lambda.N; j++)
		fprintf(fichier, "%g\t", Lambda.Pi[j]);

	fprintf(fichier, "\n");

	fprintf(fichier, "\n\nVector Phi :\n\n");

	for(j = 0; j < Lambda.N; j++)
		fprintf(fichier, "%g\t", Lambda.Phi[j]);

	fprintf(fichier, "\n");
	fclose(fichier);
	return;
}

// Forward algorithm
void Forward(HMM Lambda, int O[MaxWatchedMovies], int t_bar, long double Alpha[MaxWatchedMovies][MaxStates]){
	int i, j, t;
	long double som_alpha;


	for(j = 0; j < Lambda.N; j++)
	{
		Alpha[0][j] = Lambda.B[j][O[0]] * Lambda.Pi[j];			
	}

	for(t = 0; t < t_bar - 1; t++)
	{

		for(j = 0; j < Lambda.N; j++)
		{

			som_alpha = 0.0;

			for(i = 0; i < Lambda.N; i++)
			{
				som_alpha += Alpha[t][i] * Lambda.A[i][j];
			}
			
			Alpha[t+1][j] = som_alpha * Lambda.B[j][O[t+1]];		
		}

	}

}


// Backward algorithm
void Backward(HMM Lambda, int O[MaxWatchedMovies], int t_bar, long double Beta[MaxWatchedMovies][MaxStates], int T){
	int i, j, t;
	long double som_beta;



	for(i = 0; i < Lambda.N; i++)
	{
		Beta[T-1][i] = 1;				
	}

	for(t = T - 1; t >= t_bar; t--)
	{
		for(i = 0; i < Lambda.N; i++)
		{
			som_beta = 0.0;
			for(j = 0; j < Lambda.N; j++)
			{
				som_beta += Lambda.A[i][j] * Lambda.B[j][O[t]] * Beta[t][j];
			}
			
			Beta[t-1][i] = som_beta;			

		}
	}
}


// Forward_Backward algorithm
double Forward_Backward(HMM Lambda, int O[MaxWatchedMovies], int t_bar, int T){
    int j;
    double Proba_Observ = 0.0;
	long double Alpha[MaxWatchedMovies][MaxStates], Beta[MaxWatchedMovies][MaxStates];
	
	Forward(Lambda,O,t_bar,Alpha);
	Backward(Lambda,O,t_bar,Beta,T);
    for(j = 0; j < Lambda.N; j++)
	{

		Proba_Observ += (double)(Alpha[t_bar][j] * Beta[t_bar][j]);
	}

	return Proba_Observ;
}


// Computes Xi
void CalculXi(HMM Lambda, int O[MaxWatchedMovies], int T, long double Alpha[MaxWatchedMovies][MaxStates], long double Beta[MaxWatchedMovies][MaxStates], double Proba_Observ, long double Xi[MaxWatchedMovies][MaxStates][MaxStates]){
	int i, j, t;

	for(t = 0; t < T - 1; t++)
	{
		for(i = 0; i < Lambda.N; i++)
		{
			for(j = 0; j < Lambda.N; j++)
			{
				Xi[t][i][j] = Alpha[t][i] * Lambda.A[i][j] * Lambda.B[j][O[t + 1]] * Beta[t + 1][j];
				/* Pour eviter une division par zero */
				Xi[t][i][j] /= (Proba_Observ + MinDouble);
			}
		}
	}
}


// Computes Gamma
void CalculGamma(HMM Lambda, int O[MaxWatchedMovies], int T, long double Alpha[MaxWatchedMovies][MaxStates], long double Beta[MaxWatchedMovies][MaxStates], double Proba_Observ, long double Gamma[MaxWatchedMovies][MaxStates]){
	int j, t;

	for(t = 0; t < T; t++)
	{
		for(j = 0; j < Lambda.N; j++)
		{
			/* Rabiner Eq. 27 */	
			Gamma[t][j] = Alpha[t][j] * Beta[t][j];
			/* Pour eviter une division par zero */
			Gamma[t][j] /= (Proba_Observ + MinDouble);
		}
	}
}

// Copy of a matrix
void Copy_Matrix(double M1[MaxStates][MaxStates], double M2[MaxStates][MaxStates], int NbRow, int NbCol){
	int i, j;
	for(i = 0; i < NbRow; i++)
	{
		for(j = 0; j < NbCol; j++)
		{
			M1[i][j] = M2[i][j];
		}
	}
	return;
}


// Computes the stationnary distribution of a HMM
int StationaryDistribution(HMM * Lambda){
	int i,j,n, k,nb;
	double A_tem[MaxStates][MaxStates], A_Product[MaxStates][MaxStates];
	
	n = (*Lambda).N;
	Copy_Matrix(A_tem, (*Lambda).A,n,n);
	nb = 0;
	do
	{
		for(i = 0; i < n; i++)
		{
			for(j = 0; j < n; j++)
			{
				A_Product[i][j] = 0.0;
				for(k = 0; k < n; k++)
				{
					A_Product[i][j] += A_tem[i][k] * (*Lambda).A[k][j];
				}
			}
		}
		Copy_Matrix(A_tem, A_Product, n, n);
		nb++;
	}while(nb <= 100);

	for(i = 0; i < n; i++)
	{
		(*Lambda).Phi[i] = A_Product[0][i];
	}
	
	return j;
}


// Baum_Welch algorithm
int Baum_Welch_Mono(HMM Lambda_Init, int O[MaxWatchedMovies], int T, double epsilon, int MaxIterations, HMM * Lambda){
    HMM Lambda_Old, Lambda_New;
   int i, j, t, t_bar,l, iterations;
	double Proba_Old, Proba_New;
	long double Alpha[MaxWatchedMovies][MaxStates],  Beta[MaxWatchedMovies][MaxStates], Alpha_bar[MaxWatchedMovies][MaxStates],  Beta_bar[MaxWatchedMovies][MaxStates], Gamma[MaxWatchedMovies][MaxStates],Xi[MaxWatchedMovies][MaxStates][MaxStates];
	long double som_xi, som_gamma, s;
	
	CopyHMM(&Lambda_Old,Lambda_Init); 
	
	Forward(Lambda_Old,O,T,Alpha);
	Backward(Lambda_Old,O,1,Beta,T);
	
	t_bar = rand()%(T-1);
		
	Proba_Old = 0.0;
	for(j = 0; j < Lambda_Old.N; j++)
	{
		Proba_Old += (double)(Alpha[t_bar][j] * Beta[t_bar][j]);
	}
	Proba_New = Proba_Old;	
			
	iterations = 0;					
	do
	{
		iterations++;

		Proba_Old = Proba_New;	

		CalculGamma(Lambda_Old, O, T, Alpha, Beta, Proba_Old, Gamma);	
		CalculXi(Lambda_Old, O, T, Alpha, Beta, Proba_Old, Xi);						

		Lambda_New.N = Lambda_Old.N;
		Lambda_New.M = Lambda_Old.M;
		
        for(i = 0; i < Lambda_Old.N; i++)			
		{
			Lambda_New.Pi[i] = (double)Gamma[0][i];
		}

		for(i = 0; i < Lambda_Old.N; i++)			
		{
			for(j = 0; j < Lambda_Old.N; j++)
			{
				som_xi = 0.0;
				som_gamma = 0.0;

				for(t = 0; t < T - 1; t++)
				{
					som_xi += Xi[t][i][j];
					som_gamma += Gamma[t][i];
				}
				Lambda_New.A[i][j] = (double)(som_xi / (som_gamma + MinDouble));
			}

		}
	
		for(j = 0; j < Lambda_Old.N; j++)				
		{

			for(l = 0; l < Lambda_Old.M; l++)
			{
				s = 0.0;
				som_gamma = 0.0;
				
				for(t = 0; t < T; t++)
				{
					if(O[t] == l)
					{
						s += Gamma[t][j];
					}
				}


				for(t = 0; t < T; t++)
				{
					som_gamma += Gamma[t][j];
				}
				Lambda_New.B[j][l] = (double)(s / (som_gamma + MinDouble));
			}
		}

		Forward(Lambda_New,O,T,Alpha_bar);
		Backward(Lambda_New,O,1,Beta_bar,T);
		
		Proba_New = 0.0;
		for(j = 0; j < Lambda_New.N; j++)
		{
			Proba_New += (double)(Alpha_bar[t_bar][j] * Beta_bar[t_bar][j]);
		}

		if((Proba_New - Proba_Old) > epsilon)
		{
			CopyHMM(&Lambda_Old,Lambda_New);
			for(t = 0; t < T; t++)
			{
				for(i = 0; i < Lambda_New.N; i++)
				{
					Alpha[t][i] = Alpha_bar[t][i]; 
					Beta[t][i] = Beta_bar[t][i];
				}
			}
		}
	}while(((Proba_New - Proba_Old) > epsilon) && iterations <= MaxIterations);

	AdjustHMM(&Lambda_New); 
	CleanHMM(&Lambda_New); 

	StationaryDistribution(&Lambda_New);

	CopyHMM(Lambda,Lambda_New);
	
	return iterations;

}


// Euclidean distance
double DistanceEuclidienne(double X[MaxFeatures*MaxSymbols], double Y[MaxFeatures*MaxSymbols], int N){
	double x,y,D;
	int i;
	
	D = 0.0;
	for(i=0;(i < N);i++){
		x = X[i];
		y = Y[i];
		D += (x-y)*(x-y);
	}
	D = sqrt(D);
	return D;
}


// Manhattan distance
double DistanceManhattan(double X[MaxFeatures*MaxSymbols], double Y[MaxFeatures*MaxSymbols], int N){
	double x,y,D;
	int i;
	
	D = 0.0;
	for(i=0;(i < N);i++){
		x = X[i];
		y = Y[i];
		D += fabs(x-y);
	}
	return D;
}


// Computes the k-th component of the vector associated with a HMM
double ProbaSymboleLongTerme(int k, HMM Lambda){
	double E;
	int i;
	
	E = 0.0;
	for(i=0;(i < Lambda.N);i++){
		E += Lambda.Phi[i]*Lambda.B[i][k];
	}
	return E;
}

// Computes the vector associated with a HMM
void VecteurHMM(HMM Lambda, double V[MaxSymbols]){
	int k;
	
	for(k=0;(k < Lambda.M);k++){
		V[k] = ProbaSymboleLongTerme(k,Lambda);
		
		if(V[k] < MinFloat){
			V[k] = 0.0;
		}
	}
	return;
}

