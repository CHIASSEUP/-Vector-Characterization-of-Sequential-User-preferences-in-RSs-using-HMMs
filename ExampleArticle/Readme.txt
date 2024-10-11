
#####  Directory 'ExampleArticle'  #####
__________________________________________________________________________________________________

This directory contains the codes and the data of the example dataset presented in 
the Table 2 of [1] where three user u, v and w watched a total of seven movies.
__________________________________________________________________________________________________

==Input files:

1- The file 'users.txt': 
user_id(movie1_id,score1)(movie2_id,score2)...(movieN_id,scoreN)

2- The file 'movies.txt': 
movie_id,release_date,run_time,original_language,vote_average,vote_count,{genre1,...,genreN},#title#

3- The file 'FrequencyOfGenres.txt': 
genre_name,genre_id,genre_frequency

4- The file 'FrequencyOfLanguages.txt': 
language_name,language_id,language_frequency
__________________________________________________________________________________________________

==How to compile and run the program 

1-Compilation: 
   gcc -Wall -o RS.exe RS.c
   
2-Execution command line:
   RS.exe MoviesFile UsersFile MCsFile FrequencyOfGenresFile FrequencyOfLanguagesFile 
          NbFeatures MaxIterations Tmax SaveHMMs
		  
3-Example of execution: 
   RS.exe movies.txt users.txt MCs.txt FrequencyOfGenres.txt FrequencyOfLanguages.txt 6 100 10 0
__________________________________________________________________________________________________

==Output files

1- The file 'MCs.txt': 
delta(user_id,movie_id): (state_1,symbol_1)(state_2,symbol_2)...(state_n,symbol_n)

2- The files 'FeatureVector1.txt' to 'FeatureVector6.txt': 
user_id,[component_1,...,component_n]

3- The file 'FinalVectors.txt': 
user_id,[component_1,...,component_n]
__________________________________________________________________________________________________

[1] J. CHIASSEUP KEUDJEU, O. KENGNI NGANGMO, SYLVAIN ILOGA and T. BOUETOU BOUETOU,
    "Vector Characterization of Sequential User Preferences in Recommendation Systems using HMMs",
	IEEE Access, pp. 1-17, 2024.
__________________________________________________________________________________________________
