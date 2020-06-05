/**
 * particle_filter.cpp
 *
 * Created on: Dec 12, 2016
 * Author: Tiffany Huang
 */

#include "particle_filter.h"

#include <math.h>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <numeric>
#include <random>
#include <string>
#include <vector>

#include "helper_functions.h"

using std::string;
using std::vector;
using std::normal_distribution;
using namespace std;


std::default_random_engine gen;
void ParticleFilter::init(double x, double y, double theta, double std[]) {
  /**
   * TODO: Set the number of particles. Initialize all particles to 
   *   first position (based on estimates of x, y, theta and their uncertainties
   *   from GPS) and all weights to 1. 
   * TODO: Add random Gaussian noise to each particle.
   * NOTE: Consult particle_filter.h for more information about this method 
   *   (and others in this file).
   */
      
    if(is_initialized)
      return;
    normal_distribution<double> dist_x(x, std[0]);
    normal_distribution<double> dist_y(y, std[1]);
    normal_distribution<double> d_theta(theta, std[2]);

    num_particles = 5; 
  // TODO: Set the number of particles
  
    
 for(int i=0; i<num_particles; i++)
 { Particle newly;
   newly.id=i;
   newly.weight=1;
   newly.x=dist_x(gen);;
   newly.y=dist_y(gen);
   newly.theta=d_theta(gen);
   particles.push_back(newly);
 }
  is_initialized=true; 
   
   
}

void ParticleFilter::prediction(double delta_t, double std_pos[], 
                                double velocity, double yaw_rate) {
  /**
   * TODO: Add measurements to each particle and add random Gaussian noise.
   * NOTE: When adding noise you may find std::normal_distribution 
   *   and std::default_random_engine useful.
   *  http://en.cppreference.com/w/cpp/numeric/random/normal_distribution
   *  http://www.cplusplus.com/reference/random/default_random_engine/
   */
  for( int i=0; i<num_particles; i++)
  {
    if(fabs(yaw_rate) < 0.00001)
        {
            particles[i].x += velocity * delta_t * cos(particles[i].theta);
            particles[i].y += velocity * delta_t * sin(particles[i].theta);
        }
    else
    {
    particles[i].x+=(velocity/yaw_rate)*(sin(particles[i].theta+yaw_rate*delta_t)-sin(particles[i].theta));
    particles[i].y+=(velocity/yaw_rate)*(cos(particles[i].theta)-cos(particles[i].theta+yaw_rate*delta_t));
    particles[i].theta+=yaw_rate*delta_t;
    normal_distribution<double> dist_x(0, std_pos[0]);
    normal_distribution<double> dist_y(0, std_pos[1]);
    normal_distribution<double> dist_theta(0, std_pos[2]);
    
    particles[i].x+=dist_x(gen);
    particles[i].y+=dist_y(gen);
    particles[i].theta+=dist_theta(gen);
    }
}
}

void ParticleFilter::dataAssociation(vector<LandmarkObs> predicted, 
                                     vector<LandmarkObs>& observations) {
  /**
   * TODO: Find the predicted measurement that is closest to each 
   *   observed measurement and assign the observed measurement to this 
   *   particular landmark.
   * NOTE: this method will NOT be called by the grading code. But you will 
   *   probably find it useful to implement this method and use it as a helper 
   *   during the updateWeights phase.
   */
int q;
for(unsigned int i=0; i<observations.size(); i++)
{ 
  double min_dis=dist(observations[i].x, observations[i].y, predicted[0].x, predicted[0].y);
  for(unsigned int j=0; j<predicted.size(); j++)
  { double d=dist(observations[i].x, observations[i].y, predicted[j].x, predicted[j].y);
    if(fabs(d)<fabs(min_dis))
    {
      q=predicted[i].id;
      min_dis=d;
    }
}
  observations[i].id=q;
}
}
  
  

void ParticleFilter::updateWeights(double sensor_range, double std_landmark[], 
                                   const vector<LandmarkObs> &observations, 
                                   const Map &map_landmarks) {
  /**
   * TODO: Update the weights of each particle using a mult-variate Gaussian 
   *   distribution. You can read more about this distribution here: 
   *   https://en.wikipedia.org/wiki/Multivariate_normal_distribution
   * NOTE: The observations are given in the VEHICLE'S coordinate system. 
   *   Your particles are located according to the MAP'S coordinate system. 
   *   You will need to transform between the two systems. Keep in mind that
   *   this transformation requires both rotation AND translation (but no scaling).
   *   The following is a good resource for the theory:
   *   https://www.willamette.edu/~gorr/classes/GeneralGraphics/Transforms/transforms2d.htm
   *   and the following is a good resource for the actual equation to implement
   *   (look at equation 3.33) http://planning.cs.uiuc.edu/node99.html
   */
  double sum=0;
  for(int i = 0; i < num_particles; i++) 
  {
		double paricle_x = particles[i].x;
		double paricle_y = particles[i].y;
		double paricle_theta = particles[i].theta;
		vector<LandmarkObs> predictions;
		//Each map landmark for loop
		for(unsigned int j = 0; j < map_landmarks.landmark_list.size(); j++)
        {
//Get id and x,y coordinates
			float lm_x = map_landmarks.landmark_list[j].x_f;
			float lm_y = map_landmarks.landmark_list[j].y_f;
			int lm_id = map_landmarks.landmark_list[j].id_i;
 			if(fabs(lm_x - paricle_x) <= sensor_range && fabs(lm_y - paricle_y) <= sensor_range) 
            {
			predictions.push_back(LandmarkObs{ lm_id, lm_x, lm_y });
  			}
    	}
  vector<LandmarkObs> new_obs;
  // convert observations to mapscale
    for(unsigned int j=0; i<observations.size(); i++)
    {
			double x_m=particles[i].x+cos(particles[i].theta)*observations[j].x-sin(particles[i].theta)*observations[j].y;
      		double y_m=particles[i].y+sin(particles[i].theta)*observations[j].x+cos(particles[i].theta)*observations[j].y;
  			new_obs.push_back(LandmarkObs{observations[j].id, x_m, y_m});
  
    }
  
  dataAssociation(predictions, new_obs);
   
  for(unsigned int j=0; j<observations.size(); j++)
  { double mu_x, mu_y, w, exponent, gauss_norm;
    for(int k=0; k<predictions.size(); k++)
    {
      if(observations[j].id==predictions[k].id)
      {
        mu_x=predictions[k].x;
        mu_y=predictions[k].y;
      }
     }
  exponent = (pow(observations[j].x - mu_x, 2) / (2 * pow(std_landmark[0], 2)))
               + (pow(observations[j].y - mu_y, 2) / (2 * pow(std_landmark[1], 2)));
  gauss_norm = 1 / (2 * M_PI * std_landmark[0] * std_landmark[1]);   
  w=gauss_norm*exp(-exponent);    
  particles[i].weight*=w;
  }
    
  sum+=particles[i].weight;
    }

  
  
}

void ParticleFilter::resample() {
  /**
   * TODO: Resample particles with replacement with probability proportional 
   *   to their weight. 
   * NOTE: You may find std::discrete_distribution helpful here.
   *   http://en.cppreference.com/w/cpp/numeric/random/discrete_distribution*/

   
        //resampling
   vector<double> weights;
    double max_W;

// Calculating the max weights.
    for(int i = 0; i < num_particles; i++) {
        weights.push_back(particles[i].weight);
    }
  max_W=weights[0];
  for(int i=0; i<num_particles; i++)
  {
    if(weights[i]>max_W)
      
    	{	max_W=weights[i];
    	}   
    }
  
    
    uniform_real_distribution<double> distDouble(0, max_W);
    uniform_int_distribution<int> distInt(0, num_particles - 1);
    int index = distInt(gen);
    double beta = 0.0;
    vector<Particle> resampledParticles;
    for(int i = 0; i < num_particles; i++) {
        beta = beta + distDouble(gen) * 2.0;
        while(beta > weights[index]) {
            beta = beta - weights[index];
            index = (index + 1) % num_particles;
        }
        resampledParticles.push_back(particles[index]);
    }
    
    particles = resampledParticles;



    }

void ParticleFilter::SetAssociations(Particle& particle, 
                                     const vector<int>& associations, 
                                     const vector<double>& sense_x, 
                                     const vector<double>& sense_y) {
  // particle: the particle to which assign each listed association, 
  //   and association's (x,y) world coordinates mapping
  // associations: The landmark id that goes along with each listed association
  // sense_x: the associations x mapping already converted to world coordinates
  // sense_y: the associations y mapping already converted to world coordinates
  particle.associations= associations;
  particle.sense_x = sense_x;
  particle.sense_y = sense_y;
}

string ParticleFilter::getAssociations(Particle best) {
  vector<int> v = best.associations;
  std::stringstream ss;
  copy(v.begin(), v.end(), std::ostream_iterator<int>(ss, " "));
  string s = ss.str();
  s = s.substr(0, s.length()-1);  // get rid of the trailing space
  return s;
}

string ParticleFilter::getSenseCoord(Particle best, string coord) {
  vector<double> v;

  if (coord == "X") {
    v = best.sense_x;
  } else {
    v = best.sense_y;
  }

  std::stringstream ss;
  copy(v.begin(), v.end(), std::ostream_iterator<float>(ss, " "));
  string s = ss.str();
  s = s.substr(0, s.length()-1);  // get rid of the trailing space
  return s;
}
