#ifndef ROBO_H
#define ROBO_H
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Motion functions
void shoulder1Add(void);
void shoulder1Subtract(void);
void shoulder2Add(void);
void shoulder2Subtract(void);
void shoulder3Add(void);
void shoulder3Subtract(void);
void shoulder4Add(void);
void shoulder4Subtract(void);
void lat1Raise(void);
void lat1Lower(void);
void lat2Raise(void);
void lat2Lower(void);
void elbow1Add(void);
void elbow1Subtract(void);
void elbow2Add(void);
void elbow2Subtract(void);
void RotateAdd(void);
void RotateSubtract(void);
void MechTiltAdd(void);
void MechTiltSubtract(void);
void RaiseLeg1Forward(void);
void LowerLeg1Backwards(void);
void RaiseLeg1Outwards(void);
void LowerLeg1Inwards(void);
void RaiseLeg2Forward(void);
void LowerLeg2Backwards(void);
void RaiseLeg2Outwards(void);
void LowerLeg2Inwards(void);
void Heel1Add(void);
void Heel1Subtract(void);
void Heel2Add(void);
void Heel2Subtract(void);
void Ankle1Add(void);
void Ankle1Subtract(void);
void Ankle2Add(void);
void Ankle2Subtract(void);
void FireCannon(void);
void TurnRight(void);
void TurnLeft(void);
void TurnForwards(void);
void TurnBackwards(void);
void LightTurnRight(void);
void LightTurnLeft(void);
void LightForwards(void);
void LightBackwards(void);
void Toggle(void);

// Init and draw
void myinit(void);
void display_scene(void);
void myReshape(int w, int h);
void animation(void);

#ifdef __cplusplus
}
#endif
#endif // ROBO_H
