#ifndef GHOST_H
#define GHOST_H

class Ghost
{
public:
    bool edible;
    int  edible_max_time;
    int  edible_timer;
    bool eaten;
    bool transporting;
    float color[3];
    double speed;
    double max_speed;
    bool in_jail;
    int  jail_timer;
    double angle;
    double x, y;

    Ghost(double tx, double ty);
    ~Ghost(void);

    void Move();
    void Update(void);
    void Chase(double px, double py, bool *open_move);
    bool Catch(double px, double py);
    void Reinit(void);
    void Vulnerable(void);
    void Draw(void);
    void game_over(void);
};

#endif // GHOST_H
