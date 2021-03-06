/**
 * \file
 * Do "brute force" integration of the Bloch equation.
 */

#ifndef BRUTEFORCE_H_
#define BRUTEFORCE_H_

#include <fstream>
#include <string>
#include <vector>
#include <map>

#include <boost/numeric/odeint.hpp>
#include <boost/math/special_functions/fpclassify.hpp>
#include "interpolation.h"

/**
 * Bloch equation integrator.
 *
 * Create this class to do "brute force" tracking of your particle's spin in magnetic fields along its track.
 * It starts tracking the spin by integrating the Bloch equation when the absolute magnetic field drops below TBFIntegrator::Bmax and stops it when the field rises above this value again.
 * Then it calculates the spin flip probability after such a low-field-pass.
 */
struct TBFIntegrator{
private:
	typedef double value_type; ///< define floating point type for spin integration
	typedef std::vector<value_type> state_type; ///< define type which contains spin state vector
	typedef boost::numeric::odeint::runge_kutta_dopri5<state_type, value_type> stepper_type; ///< define integration stepper type
	typedef	boost::numeric::odeint::controlled_runge_kutta<stepper_type> controlled_stepper_type;
	typedef boost::numeric::odeint::dense_output_runge_kutta<controlled_stepper_type> dense_stepper_type;
	state_type I_n; ///< Spin vector
	double startBField[3]; /// the Bfield at the start of the spin integration (used for wL calculation)
//	stepper_type stepper;

	value_type gamma; ///< Particle's gyromagnetic ration
	std::string particlename; ///< Name of particle whose spin is to be tracked, needed for logging.
	double Bmax; ///< Spin tracking is only done when absolut magnetic field drops below this value.
	std::vector<double> BFtimes; ///< Pairs of absolute time in between which spin tracking shall be done.
	double BFBminmem; ///< Stores minimum field during one spin track for information
	bool spinlog; ///< Should the tracking be logged to file?
	double spinloginterval; ///< Time interval between log file entries.
	double nextspinlog; ///< Time when the next write to the spinlog should happen
	long int intsteps; ///< Count integrator steps during spin tracking for information.
	std::ofstream &fspinout; ///< file to log into
	double starttime; ///< time of last integration start
	double wLstarttime; ///< time when the wL calculation was started
	double wL; ///< larmor precession frequency 
	double blochPolar; ///< the projection of the spin onto the magnetic field
	double startpol; ///< user specified starting polarization of the neutrons
	
	alglib::spline1dinterpolant Binterpolant[3];
public:
	/**
	 * Constructor.
	 *
	 * Set initial values and read options from config file.
	 *
	 * @param agamma Gyromagnetic ration of particle whose spin is to be tracked.
	 * @param aparticlename Particle name.
	 * @param conf Option map containing particle specific spin tracking options.
	 * @param spinout Stream to which spin track is written
	 */
	TBFIntegrator(double agamma, std::string aparticlename, std::map<std::string, std::string> &conf, std::ofstream &spinout);
private:
	/**
	 * Do cubic spline interpolation of magnetic field components with coefficients determined in TBFderivs::TBFderivs
	 *
	 * @param t Time
	 * @param B Magnetic field components
	 */
	void Binterp(value_type t, value_type B[3]);

	/**
	 * Log integration step to file
	 *
	 * @param y1 previous state vector of the ODE system (spin)
	 * @param x1 previous time 
	 * @param y2 Current state vector of the ODE system
	 * @param x2 Current time
	 */
	void LogSpin(value_type x1, const state_type &y1, const double pv1[3], const double E1[3], const double dE1[3][3],
	 value_type x2, const state_type &y2, const double pv2[3], const double E2[3], const double dE2[3][3]);

	/**
	 * Calculate Larmor precession frequency
	 *
	 * @param x1 step start time
	 * @param y1 start spin vector
	 * @param x2 step end time
	 * @param y2 end spin vector
	 *
	 * @return Larmor frequency
	 */
	double LarmorFreq(value_type x1, const state_type &y1, value_type x2, const state_type &y2);

public:
	/**
	 *  Bloch equation integrator calls TBFIntegrator(x,y,dydx) to get derivatives
	 *
	 *  @param y Spin vector
	 *  @param dydx Returns temporal derivative of spin vector
	 *  @param x Time
	 */
	void operator()(state_type y, state_type &dydx, value_type x);
	
	/**
	 * This function will return the larmor frequency. 
	 * Used so that larmor frequency can be logged in the endlog. 
	 *
	 * @return wL, the larmor precession frequency. 
	 */
	double getLarmorFreq();
	
	/**
	 * Returns the projection of the spin onto the magnetic field at time x1.
	 *
	 * @return blochPolar 
	 */
	 double getBlochPolar();
	
	/**
	 * Track spin between two particle track points.
	 *
	 * Checks if spin should be tracked between given track points. Returns spin flip probability when tracking is finished.
	 *
	 * @param x1 Time at first track point.
	 * @param y1 State vector at first track point.
	 * @param dy1dx Temporal derivative of state vector at first track point.
	 * @param B1 Magnetic field at first track point.
	 * @param E1 Electric field at first track point.
	 * @param x2 Time at second track point.
	 * @param y2 State vector at second track point.
	 * @param dy2dx Temporal derivative of state vector at second track point.
	 * @param B2 Magnetic field at second track point.
	 *
	 * @return Probability, that NO spin flip occured (usually close to 1).
	 */
	long double Integrate(double x1, double y1[6], double dy1dx[6], double B1[4][4], double E1[3], double dE1[3][3],
						double x2, double y2[6], double dy2dx[6], double B2[4][4], double E2[3], double dE2[3][3]);

};

#endif // BRUTEFORCE_H_
