/* Author:  Uyen Truong
   Copyright (C) 2018, utplay v0.1 */

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <assert.h>
#include <SDL.h>
#include <SDL_mixer.h>

typedef struct songs
{
    char *title;
    struct songs *next;
} Songs;

typedef struct playlist
{
    Songs *head;
    int amount;
} Playlist;

Songs    *new_song(char *title);
Playlist *create_playlist(void);
Songs *nth_remove(Playlist *list, int n);
Playlist *shuffle(Playlist *list);
void      prepend(Playlist **ptr_list, Songs *song);
void      append(Playlist **ptr_list, Songs *song);
void      print_list(Playlist *list);
void      list_free(Playlist *list);
void      main_menu(Playlist *list);
void      info(int n);
void      play(Playlist *list);

int main(void)
{
    // Create playlist from album
    Playlist *list = create_playlist();
    main_menu(list);

    list_free(list);
    return 0;
}

Songs *new_song(char *title)
{
    Songs *p = malloc(sizeof(struct songs));

    p->title = title;
    p->next  = NULL;

    return p;
}

Playlist *create_playlist(void)
{
    Playlist *list = malloc(sizeof(struct playlist));
    list->head     = NULL;
    list->amount   = 0;

    DIR *d;
    struct dirent *dir;
    char *check_name;
    d = opendir("./album");

    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            check_name = strstr(dir->d_name, ".mp3");
            if (check_name != NULL)
            {
                check_name = malloc(strlen("./album/") + strlen(dir->d_name) + 1);
                strcpy(check_name, "./album/");
                strcat(check_name, dir->d_name);
                append(&list, new_song(check_name));
            }
        }
        closedir(d);
    }
    return list;
}

Songs *nth_remove(Playlist *list, int n)
{
    Songs *p = list->head;

    assert(p != NULL);
    assert(n >= 0 && n <= list->amount);

    if (list->amount == 1 && n == 0)
    {
        list->amount--;
        list->head = NULL;
        return p;
    }

    if (n == 0)
    {
        list->head = p->next;
        list->amount--;
    }
    else if (n > 0 && n < list->amount)
    {
        for (int i = 0; i < n - 1; i++)
            p = p->next;

        Songs *ptr = p->next;
        p->next = p->next->next;
        list->amount--;
        return ptr;
    }
    return p;
}

void prepend(Playlist **ptr_list, Songs *song)
{
    Playlist *p = *ptr_list;
    song->next = p->head;
    p->head = song;
    p->amount++;
}

Playlist *shuffle(Playlist *list)
{
    Playlist *new = malloc(sizeof(Playlist));
    new->head   = NULL;
    new->amount = 0;

    time_t t;
    srand((unsigned) time(&t));

    while (list->head != NULL)
    {
        int rand_val = rand() % list->amount;
        Songs *n = nth_remove(list, rand_val);

        prepend(&new, n);
    }
    print_list(new);
    return new;
}


void append(Playlist **ptr_list, Songs *song)
{
    Playlist *temp = *ptr_list;

    if (temp->head == NULL)
    {
        temp->head = song;
        temp->amount++;
    }
    else
    {
        Songs *p = temp->head;

        while (p->next != NULL)
            p = p->next;

        p->next = song;
        song->next = NULL;
        temp->amount++;
    }
}

void print_list(Playlist *list)
{
    printf("This playlist has %d songs.\n", list->amount);
    if (list->head != NULL)
    {
        for (Songs *p = list->head; p != NULL; p = p->next)
            printf("Song title %.64s\n", p->title + 8);
    }
    else
        printf("This playlist is empty.\n");
}

void list_free(Playlist *list)
{
    for (Songs *p = list->head; p != NULL; p = list->head)
    {
        list->head = list->head->next;
        list->amount--;
        free(p->title); // Free check_name
        free(p);
    }
    free(list);
}

void main_menu(Playlist *list)
{
    char cmd[10];
    int check = 1;

    while (check)
    {
        printf("Type your command : (p)lay , (h)elp, (v)ersion, "
               "(s)huffle, (l)ist, (q)uit > ");
        scanf("%s", cmd);
        if (cmd[0] == 'q' || cmd[0] == 'Q')
            check = 0;
        else if (cmd[0] == 'h' || cmd[0] == 'H')
            info(0);
        else if (cmd[0] == 'v' || cmd[0] == 'V')
            info(1);
        else if (cmd[0] == 'l' || cmd[0] == 'L')
            print_list(list);
        else if (cmd[0] == 's' || cmd[0] == 'S')
            list = shuffle(list);
        else
        {
            printf("Playing songs from playlist\n");
            print_list(list);
            play(list);
        }
    }
    printf("Thank you for coming! Goodbye!\n");
}

void info(int n)
{
    if (n == 0) // Print help menu
    {
        printf("HELP\n"
               "Follow these commands to excute your desire action:\n"
               "(h)elp   \tShow help menu\n"
               "(v)ersion\tShow the latest version of utplay\n"
               "(l)ist   \tList the song titles of current playlist\n"
               "(s)huffle\tShuffle the current playlist\n"
               "(p)lay   \tStart playing the 1st song from the playlist\n"
               "\t(P)ause \tPause the current playing song\n"
               "\t(R)esume\tResume playing the paused song\n"
               "\t(B)ack  \tBack to the previous song\n"
               "\t(N)ext  \tJump to the next song\n"
               "\t(S)top  \tStop the music player, back to main menu\n"
               "(q)uit   \tQuit the program\n");
    }
    else // Print the latest version
    {
        printf("utplay v0.1\n"
               "Copyright (C) 2018 Uyen Truong\n"
               "Contact me via haiuyentruong(at)gmail(dot)com if found bugs\n");
    }
}

void play(Playlist *list)
{
    Mix_Music *music;
    char buf[10];
    Songs *prev_song = NULL;
    Songs *song      = list->head;
    int does_stop    = 0;

    // Start SDL with audio support
    if (SDL_Init(SDL_INIT_AUDIO) == -1)
    {
        printf("SDL_Init: %s\n", SDL_GetError());
        exit(1);
    }

    // Open 44.1KHz, signed 16bit, system byte order,
    // stereo audio, using 1024 byte chunks
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) == -1)
    {
        printf("Mix_OpenAudio: %s\n", Mix_GetError());
        exit(2);
    }

    while (song != NULL)
    {
        if (does_stop) // Stop the music player
            break;

        // Load the MP3 file "music.mp3" to play as music
        music = Mix_LoadMUS(song->title);
        if (!music)
            printf("Mix_LoadMUS(): %s\n", Mix_GetError());

        // Play music forever
        if (Mix_PlayMusic(music, 0) == -1)
            printf("Mix_PlayMusic: %s\n", Mix_GetError());

        printf("Playing %.64s\n", song->title + 8);
        while (Mix_PlayingMusic())
        {
            int move_back = 0;
            int check = 1;
            SDL_Delay(100);
            while (check)
            {
                printf("Available commands: (p)ause, (r)esume,"
                       " (b)ack, (n)ext song, (s)top > ");
                if (!Mix_PlayingMusic())
                {
                    printf("\n");
                    check = 0;
                }
                else
                {
                    scanf("%s", buf);
                    if (buf[0] == 'p' || buf[0] == 'P')
                        Mix_PauseMusic();
                    else if (buf[0] == 'r' || buf[0] == 'R')
                        Mix_ResumeMusic();
                    else if (buf[0] == 'n' || buf[0] == 'N')
                    {
                        Mix_HaltMusic();
                        check = 0;
                    }
                    else if (buf[0] == 'b' || buf[0] == 'B')
                    {
                        Mix_HaltMusic();
                        song = prev_song;
                        move_back = 1;
                        check = 0;
                    }
                    else if ((buf[0] == 's' || buf[0] == 'S'))
                    {
                        Mix_HaltMusic();
                        does_stop = 1;
                        check = 0;
                    }

                }
            }
            if (!move_back)
            {
                prev_song = song;
                song = song->next;
            }
        }

        Mix_FreeMusic(music);
        music = NULL;
        SDL_Delay(200);
    }

    Mix_CloseAudio();
    SDL_Quit();
}
