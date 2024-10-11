
#####  Directory 'MovieLens100K' #####
__________________________________________________________________________________________________

This directory contains the codes and the data of the of the experiments realized in [1]
on the 'MovieLens-100K'.
__________________________________________________________________________________________________

==Sub-directories:

1- 'Tmax100': Contains the experimental results of the Table 11.b of [1] for (Tmax = 100) 

2- 'Tmax200': Contains the experimental results of the Table 11.b of [1] for (Tmax = 200) 
__________________________________________________________________________________________________

==Input files:

1- The file 'users.txt': 
user_id(movie1_id,score1)(movie2_id,score2)...(movieN_id,scoreN)

2- The file 'movies.txt': 
movie_id,release_date,run_time,original_language,vote_average,vote_count,{genre1,...,genreN},#title#

3- The file 'FrequencyOfGenres.txt': 
genre_name,genre_id,genre_frequency
__________________________________________________________________________________________________

==How to compile and run the program 

1-Compilation: 
   gcc -Wall -Wl,--stack,536870912 -o RS.exe RS.c
   
2-Execution command line:
   RS.exe MoviesFile UsersFile MCsFile FrequencyOfGenresFile  
          NbFeatures MaxIterations NbNeighbours MaxRecoms Tmax SaveHMMs
		  
3-Example of execution: 
   RS.exe movies.txt users.txt MCs.txt FrequencyOfGenres.txt 4 100 30 20 200 0
__________________________________________________________________________________________________

==Output files

1- The file 'MCs.txt': 
delta(user_id,movie_id): (state_1,symbol_1)(state_2,symbol_2)...(state_n,symbol_n)

2- The files 'FeatureVector1.txt' to 'FeatureVector4.txt': 
user_id,[component_1,...,component_n]

3- The file 'FinalVectors.txt': 
user_id,[component_1,...,component_n]

4- The file 'Neighbours.txt': 
user_id,[neighbor_id_1,...,neighbor_id_k] (watched_movie_1,...,watched_movie_n) --> {Number_Recommended_movies}[(recommended_movie_1,importance_1)...(recommended_movie_k,importance_k)]

5- The file 'Recommendations.txt': 
user_id,(watched_movie_1,...,watched_movie_n) --> [(recommended_movie_1,importance_1)...(recommended_movie_k,importance_k)]
__________________________________________________________________________________________________

[1] J. CHIASSEUP KEUDJEU, O. KENGNI NGANGMO, SYLVAIN ILOGA and T. BOUETOU BOUETOU,
    "Vector Characterization of Sequential User Preferences in Recommendation Systems using HMMs",
	IEEE Access, pp. 1-17, 2024.
__________________________________________________________________________________________________
