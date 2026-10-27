#ifndef DYNAMIC_CAMERA_H
#define DYNAMIC_CAMERA_H

#include <iostream>
#include <limits>

#include "camera.h"

constexpr float INF = std::numeric_limits<float>::infinity();

// AABB in 1D
struct Interval {
    float min;
    float max;

    constexpr bool contains(const Interval& other) const {
        return (min <= other.min) && (max >= other.max);
    }

    constexpr bool contains(float x) const { return (min <= x) && (x <= max); }

    // signedDistance returns a positive smallest distance of x from the interval boundary
    // if it is not contained, else it returns a smallest (in magnitude) negative distance
    // from the interval boundary.
    constexpr float signedDistance(float x) const {
        if (contains(x)) {
            // 0 or negative
            return std::max(x - max, min - x);
        }
        // positive
        return std::max(x - max, min - x);
    }
};

struct DynamicCamera {
    void update(Camera& camera,                           // the camera to update
                const sf::Vector2f& trackedPosWorld,      // world coords of player to track
                const sf::Vector2f& trackedVelocityWorld  // world velocity of player to track
    ) {
        // TODO: (HINTS/outline)
        // collect statistics on the player position and velocity in (normalized, if you like)
        // window coordinates. Compute lookAhead distance in world coords, adjusting for
        // if the player is moving left or right.

        // State of trackedVelocityWorld already accounts for direction and magnitude
        // Just convert to window percentage
        lookAhead = camera.worldToWindowPercentDist(trackedVelocityWorld).x;

        // Now update lookAhead state using an EMA over its previous state and the lookAhead world vector. 
        
        lookaheadState = alpha * lookAhead + (1.f - alpha) * lookaheadState;

        // Then determine if the player is in the dead zone, soft rejection zone, or hard rejection zone. 
        // For each, perform EMA on the delta of the camera displacement (ignoring lookahead) appropriately. 
        // Next, set the camera center by offsetting the cameraUnadjustedCenter, combining lookAhead.
        // Finally update cameraUnadjustedCenter, but without lookahead.
        
        // Get delta for player position vs camera center in normalized window units
        sf::Vector2f player_delta = camera.worldToWindowPercentDist(
            trackedPosWorld - cameraUnadjustedCenter
        );
        // Check hard/soft/dead in that order
        if( !softZoneX.contains(player_delta.x) ) {         // Beyond Hard zone
            // Hard set camera so player falls within hard zone
            sf::Vector2f camera_delta = camera.windowPercentToWorldDist({
                softZoneX.signedDistance(player_delta.x),
                0.f
            });
            camera_delta.y = cameraUnadjustedCenter.y;
            camera.translateWorld(camera_delta);
        }
        else if ( !deadZoneX.contains(player_delta.x) ){    // Beyond Soft zone

        }
        else {                                              // If not beyond Hard or Soft zone, player is within Dead zone

        }
    }

    float alpha;  // alpha to use for damping of camera position, *as well as* for smoothing of
                  // lookahead offset.
    Interval deadZoneX;  // percentage interval along x-axis to use for dead zone in [0, 1]
    // percentage interval along x-axis to use for soft rejection zone in [0, 1]
    Interval softZoneX = {-INF, INF};
    // (everything beyond softZone is considered hard zone).
    float lookAhead;  // screen-percentage lookahead
    mutable sf::Vector2f
        cameraUnadjustedCenter;      // the camera world center from last frame, ignoring lookahead
    mutable float lookaheadState{};  // the EMA of the accumulated lookahead offset in world coords
};

#endif // DYNAMIC_CAMERA_H
