#ifndef SimTK_MATTER_SUBSYSTEM_H_
#define SimTK_MATTER_SUBSYSTEM_H_

/* Portions copyright (c) 2005-6 Stanford University and Michael Sherman.
 * Contributors: Paul Mitiguy
 * 
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including 
 * without limitation the rights to use, copy, modify, merge, publish, 
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included 
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "SimTKcommon.h"
#include "simbody/internal/common.h"
#include "simbody/internal/Subsystem.h"

namespace SimTK {

/**
 * The still-abstract parent of all MatterSubsystems (such as the
 * one generated by Simbody). This is derived from Subsystem.
 *
 * The MatterSubsystem class implements a friendlier API on top of
 * the efficient, minimalist interface required of concrete MatterSubsystem
 * implementation classes (such as SimbodyMatterSubsystem). In many cases
 * these are inline implementations, but some performance
 * tradeoffs are made in this API in the interest of beauty and simplicity. These
 * are expected to be insignificant in most applications but if (after measurement!)
 * you determine that these are a bottleneck you are always welcome to call
 * directly into the part of the interface which is directly implemented by
 * the concrete class. The concrete class methods are defined after the 
 * friendly one below.
 *
 * This API was designed by Paul Mitiguy and Michael Sherman to address
 * the anticipated needs of Paul's BMI 215 students at Stanford. However,
 * we expect it will be useful for many other purposes.
 */
class SimTK_SIMBODY_EXPORT MatterSubsystem : public Subsystem {
public:
    MatterSubsystem() { }

    ///////////////////////////////
    // PAUL'S FRIENDLY INTERFACE //
    ///////////////////////////////

        // MASS PROPERTIES //

    /// Return the mass properties of body A, measured in the A frame, but expressed
    /// in body B. That is, return the mass, mass center location T_OA_CA, and
    /// the inertia about the body origin OA, expressed in B. If body B is the
    /// same body as body A, then we can obtain the mass properties without 
    /// having realized positions in the State, otherwise positions must be
    /// valid.
    /// If inBodyB==GroundId, this is the Spatial Inertia matrix as used in the
    /// Spatial Operator Algebra formulation (that is, the local body mass
    /// properties but expresed in Ground). You can pull out the
    /// individual elements of MassProperties m with m.getMass(), 
    /// m.getMassCenter() and m.getInertia(). You can see this as
    /// a Spatial Inertia Matrix with m.toSpatialMat() or as a 6x6
    /// matrix with m.toMat66().
    MassProperties calcBodyMassPropertiesInBody(const State& s, 
                                                BodyId objectBodyA,
                                                BodyId inBodyB)
    {
        const MassProperties& mp = getBodyMassProperties(s, objectBodyA);
        if (inBodyB == objectBodyA) 
            return mp;

        // must be at Stage >= Position
        Rotation R_AB = ~getBodyPosition(s,objectBodyA).R();  // R_AG (assume B==G)
        if (inBodyB != GroundId)
            R_AB *= getBodyPosition(s,inBodyB).R(); // R_AB = R_AG*R_GB
        return mp.reexpress(R_AB); // i.e., R_AB
    }

    /// Return the mass properties of body A, measured in the A frame, but
    /// expressed in Ground and converted to a Spatial Inertia Matrix
    /// @verbatim
    /// M=[      I         crossMat(m*c) ]
    ///   [ ~crossMat(m*c) diag(m)       ]
    /// @endverbatim
    SpatialMat calcBodySpatialInertiaMatrixInGround(const State&,
                                                    BodyId objectBodyA);

    Vec3 calcBodyMassCenterLocation(const State&, 
                                    BodyId objectBodyA);

    Vec3 calcBodyMassCenterLocationInBody(const State&, 
                                          BodyId      objectBodyA,
                                          BodyId      inBodyB, 
                                          const Vec3& fromLocationOnBodyB);

    /// Return the central inertia for body A, that is, the inertia taken about
    /// body A's mass center CA, but reexpressed in body B's frame.
    Inertia calcBodyCentralInertia(const State&, 
                                   BodyId objectBodyA);

    /// Return the central inertia for body A, that is, the inertia taken about
    /// body A's mass center CA, but reexpressed in body B's frame.
    Inertia calcBodyInertiaAboutBodyPoint(const State&, 
                                          BodyId      objectBodyA, 
                                          BodyId      inBodyB, 
                                          const Vec3& aboutLocationOnBodyB);


    /// Return total system mass, mass center location measured from the Ground origin,
    /// and system inertia taken about the Ground origin, expressed in Ground.
    MassProperties calcSystemMassPropertiesInGround(const State&);

    /// Return the system inertia matrix taken about the system center of mass,
    /// expressed in Ground.
    Inertia calcSystemCentralInertiaInGround(const State&);

    /// Return the location T_OG_C of the system mass center C, measured from the ground
    /// origin OG, and expressed in Ground. 
    Vec3 calcSystemMassCenterLocationInGround(const State&);

    /// Return the velocity V_G_C = d/dt T_OG_C of the system mass center C in the Ground frame G,
    /// expressed in G.
    Vec3 calcSystemMassCenterVelocityInGround(const State&);

    /// Return the acceleration A_G_C = d^2/dt^2 T_OG_C of the system mass center C in
    /// the Ground frame G, expressed in G.
    Vec3 calcSystemMassCenterAccelerationInGround(const State&);



        // POSITION //

    /// Return X_BA, the spatial transform to body A's frame from body B's frame.
    Transform calcBodyTransformInBody(const State&, 
                                      BodyId objectBodyA, 
                                      BodyId inBodyB);

    /// Return R_BA, the rotation matrix to body A's x,y,z axes from body B's x,y,z axes.
    Rotation calcBodyRotationInBody(const State&, 
                                    BodyId objectBodyA, 
                                    BodyId inBodyB);

    /// Return T_OB_OA, the location of body A's origin OA, measured from body B's origin, expressed in body B.
    Vec3 calcBodyOriginLocationInBody(const State&,
                                      BodyId objectBodyA, 
                                      BodyId inBodyB);

    /// Given a vector T_OA_P from body A's origin to a point P on body A, expressed in body A, return the vector
    /// T_OB_P from body B's origin to point P, expressed in body B.
    Vec3 calcBodyPointLocationInBody(const State&,
                                     BodyId      onBodyA,
                                     const Vec3& locationOnBodyA, 
                                     BodyId      inBodyB);

    /// Given a vector expressed in body A, return that same vector expressed in body B.
    Vec3 calcBodyVectorInBody(const State&, 
                              BodyId      onBodyA, 
                              const Vec3& vectorOnBodyA, 
                              BodyId      inBodyB);


        // VELOCITY //

    /// Return the angular and linear velocity of body A's frame in body B's frame, expressed in body B,
    /// and arranged as a SpatialVec.
    SpatialVec calcBodySpatialVelocityInBody(const State&, 
                                             BodyId objectBodyA, 
                                             BodyId inBodyB);

    /// Return the angular velocity w_BA of body A's frame in body B's frame, expressed in body B.
    Vec3 calcBodyAngularVelocityInBody(const State&, 
                                       BodyId objectBodyA, 
                                       BodyId inBodyB);

    /// Return the velocity of body A's origin point in body B's frame, expressed in body B.
    Vec3 calcBodyOriginVelocityInBody(const State&,
                                      BodyId objectBodyA,
                                      BodyId inBodyB);

    /// Return the velocity of a point P fixed on body A, in body B's frame, expressed in body B.
    Vec3 calcBodyFixedPointVelocityInBody(const State&, 
                                          BodyId      objectBodyA, 
                                          const Vec3& locationOnBodyA, 
                                          BodyId      inBodyB);

    /// Return the velocity of a point P moving on body A, in body B's frame, expressed in body B.
    Vec3 calcBodyFixedPointVelocityInBody(const State&,
                                          BodyId      objectBodyA, 
                                          const Vec3& locationOnBodyA, 
                                          const Vec3& velocityOnBodyA,
                                          BodyId      inBodyB);

        // ACCELERATION //

    /// Return the angular and linear acceleration of body A's frame in body B's frame, expressed in body B,
    /// and arranged as a SpatialVec.
    SpatialVec calcBodySpatialAccelerationInBody(const State&, 
                                                 BodyId objectBodyA, 
                                                 BodyId inBodyB);

    /// Return the angular acceleration of body A's frame in body B's frame, expressed in body B.
    Vec3 calcBodyAngularAccelerationInBody(const State&, 
                                           BodyId objectBodyA, 
                                           BodyId inBodyB);

    /// Return the acceleration of body A's origin point in body B's frame, expressed in body B.
    Vec3 calcBodyOriginAccelerationInBody(const State&, 
                                          BodyId objectBodyA, 
                                          BodyId inBodyB);

    /// Return the acceleration of a point P fixed on body A, in body B's frame, expressed in body B.
    Vec3 calcBodyFixedPointAccelerationInBody(const State&, 
                                              BodyId      objectBodyA, 
                                              const Vec3& locationOnBodyA, 
                                              BodyId      inBodyB);

    /// Return the velocity of a point P moving on body A, in body B's frame, expressed in body B.
    Vec3 calcBodyMovingPointAccelerationInBody(const State&, 
                                               BodyId       objectBodyA, 
                                               const Vec3&  locationOnBodyA, 
                                               const Vec3&  velocityOnBodyA, 
                                               const Vec3&  accelerationOnBodyA,
                                               BodyId       inBodyB);

        // SCALAR DISTANCE //

    /// Calculate the distance to a point PA on body A from a point PB on body B.
    /// We are given the location vectors T_OA_PA and T_OB_PB, expressed in their
    /// respective frames. We return |T_OB_OA|.
    Real calcPointToPointDistance(const State&,
                                  BodyId      bodyA,
                                  const Vec3& locationOnBodyA,
                                  BodyId      bodyB,
                                  const Vec3& locationOnBodyB);

    /// Calculate the time rate of change of distance from a fixed point PA on body A to a fixed point
    /// PB on body B. We are given the location vectors T_OA_PA and T_OB_PB, expressed in their
    /// respective frames. We return d/dt |T_OB_OA|, under the assumption that the time derivatives
    /// of the two given vectors in their own frames is zero.
    Real calcFixedPointToPointDistanceTimeDerivative(const State&,
                                                     BodyId      bodyA,
                                                     const Vec3& locationOnBodyA,
                                                     BodyId      bodyB,
                                                     const Vec3& locationOnBodyB);


    /// Calculate the time rate of change of distance from a moving point PA on body A to a moving point
    /// PB on body B. We are given the location vectors T_OA_PA and T_OB_PB, and the velocities of
    /// PA in A and PB in B, all expressed in their respective frames. We return d/dt |T_OB_OA|,
    /// taking into account the time derivatives of the locations in their local frames, as well
    /// as the relative velocities of the bodies.
    Real calcMovingPointToPointDistanceTimeDerivative(const State&,
                                                      BodyId      bodyA,
                                                      const Vec3& locationOnBodyA,
                                                      const Vec3& velocityOnBodyA,
                                                      BodyId      bodyB,
                                                      const Vec3& locationOnBodyB,
                                                      const Vec3& velocityOnBodyB);

    /// Calculate the second time derivative of distance from a fixed point PA on body A to a fixed point
    /// PB on body B. We are given the location vectors T_OA_PA and T_OB_PB, expressed in their
    /// respective frames. We return d^2/dt^2 |T_OB_OA|, under the assumption that the time derivatives
    /// of the two given vectors in their own frames is zero.
    Real calcFixedPointToPointDistance2ndTimeDerivative(const State&,
                                                        BodyId      bodyA,
                                                        const Vec3& locationOnBodyA,
                                                        BodyId      bodyB,
                                                        const Vec3& locationOnBodyB);

    /// Calculate the second time derivative of distance from a moving point PA on body A to a moving point
    /// PB on body B. We are given the location vectors T_OA_PA and T_OB_PB, and the velocities of
    /// PA in A and PB in B, all expressed in their respective frames. We return d^2/dt^2 |T_OB_OA|,
    /// taking into account the time derivatives of the locations in their local frames, as well
    /// as the relative velocities and accelerations of the bodies.
    Real calcMovingPointToPointDistance2ndTimeDerivative(const State&,
                                                         BodyId      bodyA,
                                                         const Vec3& locationOnBodyA,
                                                         const Vec3& velocityOnBodyA,
                                                         const Vec3& accelerationOnBodyA,
                                                         BodyId      bodyB,
                                                         const Vec3& locationOnBodyB,
                                                         const Vec3& velocityOnBodyB,
                                                         const Vec3& accelerationOnBodyB);

    //////////////////////////////
    // CONCRETE CLASS INTERFACE //
    //////////////////////////////

    // The MatterSubsystemRep (an abstract class) provides implementations underlying the MatterSubsystem
    // wrapper methods below, typically as virtual methods to be implemented by derived concrete classes 
    // (e.g. SimbodyMatterSubsystemRep). The wrappers defined below are in turn used
    // to implement the friendlier API defined above.

        // TOPOLOGY STAGE (no state) //

    int getNBodies()      const;    // includes ground, also # mobilizers+1
    int getNParticles()   const;
    int getNMobilities()  const;
    int getNConstraints() const;    // i.e., Constraint definitions (each is multiple equations)

    BodyId        getParent  (BodyId bodyNum) const;
    Array<BodyId> getChildren(BodyId bodyNum) const;

        // MODEL STAGE responses //

    const Real& getMobilizerQ(const State&, BodyId, int mobilityIndex) const;
    const Real& getMobilizerU(const State&, BodyId, int mobilityIndex) const;

        // MODEL STAGE operators //
    // none

        // MODEL STAGE solvers //

    void setMobilizerQ(State&, BodyId, int mobilityIndex, const Real& mobilityValue) const;
    void setMobilizerU(State&, BodyId, int mobilityIndex, const Real& mobilityValue) const;

        // INSTANCE STAGE responses //

    /// Return the mass, center of mass location measured from the body origin, and
    /// inertia about the body origin. Center of mass and inertia are expressed in
    /// the body frame. Individual quantities can be extracted from the MassProperties
    /// object via getMass(), getMassCenter(), and getInertia() methods.
    const MassProperties& getBodyMassProperties(const State&, BodyId) const;
    const Vector&         getParticleMasses(const State&)      const;

    const Transform&  getMobilizerFrame(const State&, BodyId) const;
    const Transform&  getMobilizerFrameOnParent(const State&, BodyId) const;

        // INSTANCE STAGE operators //
    // none

        // INSTANCE STAGE solvers //
    // none

        // TIME STAGE responses
    // none

        // TIME STAGE operators
    // none

        // TIME STAGE solvers

    /// This is a solver which sets the body's mobilizer transform as close
    /// as possible to the supplied Transform. The degree to which this is
    /// possible depends of course on the mobility provided by this body's
    /// mobilizer. However, no error will occur; on return the coordinates
    /// for this mobilizer will be as close as we can get them. Note: this
    /// has no effect on any coordinates except the q's for this mobilizer.
    /// You can call this solver at Stage::Time or higher (because there can
    /// be time-dependent constraints on position); it will
    /// leave you no higher than Stage::Time since it changes the configuration.
    void setMobilizerPosition(State&, BodyId, const Transform& X_MbM) const;

        // POSITION STAGE responses //

    const Vector_<Vec3>& getParticleLocations(const State& s) const; 

    /// Extract from the state cache the already-calculated spatial configuration of
    /// body B's body frame, measured with respect to the ground frame and expressed
    /// in the ground frame. That is, we return the location of the body frame's
    /// origin, and the orientation of its x, y, and z axes, as the transform X_GB.
    /// This response is available at Position stage.
    const Transform& getBodyPosition(const State&, BodyId) const;

    /// At stage Position or higher, return the cross-mobilizer transform.
    /// This is X_MbM, the body's inboard mobilizer frame M measured and expressed in
    /// the parent body's corresponding outboard frame Mb.
    const Transform& getMobilizerPosition(const State&, BodyId) const;

    /// This is available at Stage::Position. These are *absolute* constraint
    /// violations qerr=g(t,q), that is, they are unweighted.
    const Vector& getQConstraintErrors(const State&) const;

    /// This is the weighted norm of the errors returned by getQConstraintErrors(),
    /// available whenever this subsystem has been realized to Stage::Position.
    /// This is the scalar quantity that we need to keep below "tol"
    /// during integration.
    Real calcQConstraintNorm(const State&) const;

        // POSITION STAGE operators //


    /// Apply a force to a point on a body (a station). Provide the
    /// station in the body frame, force in the ground frame. Must
    /// be realized to Position stage prior to call.
    void addInStationForce(const State&, BodyId, const Vec3& stationInB, 
                           const Vec3& forceInG, Vector_<SpatialVec>& bodyForces) const;

    /// Apply a torque to a body. Provide the torque vector in the
    /// ground frame.
    void addInBodyTorque(const State&, BodyId, const Vec3& torqueInG, 
                         Vector_<SpatialVec>& bodyForces) const;

    /// Apply a scalar joint force or torque to an axis of the
    /// indicated body's mobilizer.
    void addInMobilityForce(const State&, BodyId, int axis, const Real& f,
                            Vector& mobilityForces) const;

        // POSITION STAGE solvers //

    /// This is a solver which sets the body's cross-mobilizer velocity as close
    /// as possible to the supplied angular and linear velocity. The degree to which this is
    /// possible depends of course on the mobility provided by this body's
    /// mobilizer, in its current configuration. However, no error will occur; on return
    /// the velocity coordinates (u's) for this mobilizer will be as close as we can get them.
    /// Note: this has no effect on any coordinates except the u's for this mobilizer.
    /// You can call this solver at Stage::Position or higher; it will
    /// leave you no higher than Stage::Position since it changes the velocities.
    void setMobilizerVelocity(State&, BodyId, const SpatialVec& V_MbM) const;

    /// This is a solver you can call after the State has been realized
    /// to stage Position. It will project the Q constraints
    /// along the error norm so that getQConstraintNorm() <= tol, and will
    /// project out the corresponding component of y_err so that y_err's Q norm
    /// is reduced. Returns true if it does anything at all to State or y_err.
    bool projectQConstraints(State&, Vector& y_err, Real tol, Real targetTol) const;

        // VELOCITY STAGE responses //

    /// Extract from the state cache the already-calculated spatial velocity of
    /// body B's body frame, measured with respect to the ground frame and expressed
    /// in the ground frame. That is, we return the linear velocity v_GB of the body
    /// frame's origin, and the body's angular velocity w_GB as the spatial velocity
    /// vector V_GB = {w_GB, v_GB}. This response is available at Velocity stage.
    const SpatialVec& getBodyVelocity(const State&, BodyId) const;

    /// At stage Velocity or higher, return the cross-mobilizer velocity.
    /// This is V_MbM, the relative velocity of the body's inboard mobilizer
    /// frame M in the parent body's corresponding outboard frame Mb, 
    /// measured and expressed in Mb. Note that this isn't the usual 
    /// spatial velocity since it isn't expressed in G.
    const SpatialVec& getMobilizerVelocity(const State&, BodyId) const;

    /// This is available at Stage::Velocity. These are *absolute* constraint
    /// violations verr=v(t,q,u), that is, they are unweighted.
    const Vector& getUConstraintErrors(const State&) const;

    /// This is the weighted norm of the errors returned by getUConstraintErrors().
    /// That is, this is the scalar quantity that we need to keep below "tol"
    /// during integration.
    Real calcUConstraintNorm(const State&) const;

        // VELOCITY STAGE operators //
    // none

        // VELOCITY STAGE solvers //

    /// This is a solver you can call after the State has been realized
    /// to stage Velocity. It will project the U constraints
    /// along the error norm so that getUConstraintNorm() <= tol, and will
    /// project out the corresponding component of y_err so that y_err's U norm
    /// is reduced.
    bool projectUConstraints(State&, Vector& y_err, Real tol, Real targetTol) const;

        // DYNAMICS STAGE responses //
    // none

        // DYNAMICS STAGE operators //
    // none

        // DYNAMICS STAGE solvers //
    // none

        // ACCELERATION STAGE responses //

    /// Extract from the state cache the already-calculated spatial acceleration of
    /// body B's body frame, measured with respect to the ground frame and expressed
    /// in the ground frame. That is, we return the linear acceleration a_GB of the body
    /// frame's origin, and the body's angular acceleration alpha_GB as the spatial acceleration
    /// vector A_GB = {alpha_GB, a_GB}. This response is available at Acceleration stage.
    const SpatialVec& getBodyAcceleration(const State&, BodyId) const;

    /// This is available at Stage::Acceleration. These are *absolute* constraint
    /// violations aerr = A udot - b, that is, they are unweighted.
    const Vector& getUDotConstraintErrors(const State&) const;

    /// This is the weighted norm of the errors returned by getUDotConstraintErrors().
    Real calcUDotConstraintNorm(const State&) const;


    ///////////////////////////////
    // MATTER SUBSYSTEM SERVICES //
    ///////////////////////////////

    // Methods below here are services provided by the MatterSubsystem for use
    // by other internal objects, such as Systems, ForceSubsystems, or concrete
    // MatterSubsystem implementations.


    /// Extract from the state cache the already-calculated spatial orientation
    /// of body B's body frame x, y, and z axes expressed in the ground frame,
    /// as the rotation matrix R_GB. This response is available at Position stage.
    const Rotation& getBodyRotation(const State& s, BodyId body) const {
        return getBodyPosition(s,body).R();
    }
    /// Extract from the state cache the already-calculated spatial location
    /// of body B's body frame origin, measured from the ground origin and
    /// expressed in the ground frame, as the translation vector T_GB.
    /// This response is available at Position stage.
    const Vec3& getBodyLocation(const State& s, BodyId body) const {
        return getBodyPosition(s,body).T();
    }

    /// Extract from the state cache the already-calculated inertial angular
    /// velocity vector w_GB of body B, measured with respect to the ground frame
    /// and expressed in the ground frame. This response is available at Velocity stage.
    const Vec3& getBodyAngularVelocity(const State& s, BodyId body) const {
        return getBodyVelocity(s,body)[0]; 
    }
    /// Extract from the state cache the already-calculated inertial linear
    /// velocity vector v_GB of body B, measured with respect to the ground frame
    /// and expressed in the ground frame. This response is available at Velocity stage.
    const Vec3& getBodyLinearVelocity(const State& s, BodyId body) const {
        return getBodyVelocity(s,body)[1];
    }

    /// Return the Cartesian (ground) location of a station fixed to a body. That is
    /// we return locationInG = X_GB * stationB. Cost is 18 flops. This operator is
    /// available at Position stage.
    Vec3 calcStationLocation(const State& s, BodyId bodyB, const Vec3& stationB) const {
        return getBodyPosition(s,bodyB)*stationB;
    }
    /// Given a station on body B, return the station of body A which is at the same location
    /// in space. That is, we return stationInA = X_AG * (X_GB*stationB). Cost is 36 flops.
    /// This operator is available at Position stage.
    Vec3 calcStationLocationInBody(const State& s, BodyId bodyB, const Vec3& stationB, BodyId bodyA) {
        return ~getBodyPosition(s,bodyA) 
                * calcStationLocation(s,bodyB,stationB);
    }
    /// Re-express a vector expressed in the B frame into the same vector in G. That is,
    /// we return vectorInG = R_GB * vectorInB. Cost is 15 flops. 
    /// This operator is available at Position stage.
    Vec3 calcVectorOrientation(const State& s, BodyId bodyB, const Vec3& vectorB) const {
        return getBodyRotation(s,bodyB)*vectorB;
    }
    /// Re-express a vector expressed in the B frame into the same vector in some other
    /// body A. That is, we return vectorInA = R_AG * (R_GB * vectorInB). Cost is 30 flops.
    /// This operator is available at Position stage.
    Vec3 calcVectorOrientationInBody(const State& s, BodyId bodyB, const Vec3& vectorB, BodyId bodyA) const {
        return ~getBodyRotation(s,bodyA) * calcVectorOrientation(s,bodyB,vectorB);
    }

    /// Given a station fixed on body B, return its inertial (Cartesian) velocity,
    /// that is, its velocity relative to the ground frame, expressed in the
    /// ground frame. Cost is 27 flops. This operator is available at Velocity stage.
    Vec3 calcStationVelocity(const State& s, BodyId bodyB, const Vec3& stationB) const {
        const SpatialVec& V_GB       = getBodyVelocity(s,bodyB);
        const Vec3        stationB_G = calcVectorOrientation(s,bodyB,stationB);
        return V_GB[1] + V_GB[0] % stationB_G; // v + w X r
    }

    /// Given a station fixed on body B, return its velocity relative to the body frame of
    /// body A, and expressed in body A's body frame. Cost is 54 flops.
    /// This operator is available at Velocity stage.
    /// TODO: UNTESTED!!
    /// TODO: maybe these between-body routines should return results in ground so that they
    /// can be easily combined. Easy to re-express vector afterwards.
    Vec3 calcStationVelocityInBody(const State& s, BodyId bodyB, const Vec3& stationB, BodyId bodyA) const {
        // If body B's origin were coincident with body A's, then Vdiff_AB would be the relative angular
        // and linear velocity of body B in body A, expressed in G. To get the point we're interested in,
        // we need the vector from body A's origin to stationB to account for the extra linear velocity
        // that will be created by moving away from the origin.
        const SpatialVec Vdiff_AB = getBodyVelocity(s,bodyB) - getBodyVelocity(s,bodyA); // 6

        // This is a vector from body A's origin to the point of interest, expressed in G.
        const Vec3 stationA_G = calcStationLocation(s,bodyB,stationB) - getBodyLocation(s,bodyA); // 21
        const Vec3 v_AsB_G = Vdiff_AB[1] + Vdiff_AB[0] % stationA_G; // 12
        return ~getBodyRotation(s,bodyA) * v_AsB_G; // 15
    }

    /// This can be called at any time after construction. It sizes a set of
    /// force arrays (if necessary) and then sets them to zero. The concrete
    /// implementations of the "addIn" operators (see above) can then be used by
    /// the force subsystems to accumulate forces.
    void resetForces(Vector_<SpatialVec>& bodyForces,
                     Vector_<Vec3>&       particleForces,
                     Vector&              mobilityForces) const 
    {
        bodyForces.resize(getNBodies());         bodyForces.setToZero();
        particleForces.resize(getNParticles());  particleForces.setToZero();
        mobilityForces.resize(getNMobilities()); mobilityForces.setToZero();
    }

        // BOOKKEEPING //
    SimTK_PIMPL_DOWNCAST(MatterSubsystem, Subsystem);
    class MatterSubsystemRep& updRep();
    const MatterSubsystemRep& getRep() const;
};

} // namespace SimTK

#endif // SimTK_MATTER_SUBSYSTEM_H_