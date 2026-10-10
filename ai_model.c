#include "ai_model.h"
#include <math.h>

static const float weight_cap  =  0.45f;
static const float weight_dist = -0.30f;
static const float weight_conf = -0.25f;
static const float bias        =  0.10f;

static const float CAP_SCALE  = 50.0f;
static const float DIST_SCALE = 10.0f;
static const float CONF_SCALE = 100.0f;

float calculate_ai_score(int capacity_diff, int room_distance, int historical_conflict_rate) {
    float x_cap  = (float)capacity_diff            / CAP_SCALE;
    float x_dist = (float)room_distance            / DIST_SCALE;
    float x_conf = (float)historical_conflict_rate / CONF_SCALE;

    float z = bias + weight_cap * x_cap + weight_dist * x_dist + weight_conf * x_conf;
    return 1.0f / (1.0f + expf(-z));
}