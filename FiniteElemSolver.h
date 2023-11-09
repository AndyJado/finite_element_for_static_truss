/*
	Written by CL Frank in 2014.6
*/
#ifndef _FINITE_ELEMENT_SOLVER_H_
#define _FINITE_ELEMENT_SOLVER_H_
#include <vector>
#include <set>
#include "Eigen/Sparse"
#include "Eigen/Dense"
#include "Vec.h"
using std::vector;
using std::pair;
using std::set;

typedef Eigen::Matrix<double,2,6> Matrix2_6d;
typedef Eigen::Matrix<double,2,4> Matrix2_4d;
typedef Eigen::Matrix<double,4,4> Matrix4d;
typedef Eigen::Matrix<double,6,6> Matrix6d;
typedef Eigen::Triplet<double> Tr;
typedef Eigen::SparseMatrix<double> SparseMatrixType;
#define BIGNUMERIC 1e16


class static_solver
{
public:
	//Young Modulus 
	double		            ex;
	//Possion Ratio
/*	double		           prxy;*/
	//constraint store the index of constraint nodes.
	vector<int>			    constraint; 
	//ext_f is the exterior force applied to nodes.
	vector<pair<int,vec> >  ext_f;
	vector<pair<int,vec2> >  ext_f2d;
	//du is the displacement vector of nodes need to solve.
	Eigen::VectorXd		    du;
	//nodes of the truss structure
	vector<vec>			    nd;
	vector<vec2>			    nd2;
	//radius array of strut unit
	vector<double>		    link_rad;
	//strut unit,contain the index of two end nodes.
	vector< pair<int,int> > un;
	//stress of strut unit
	vector<double>			stress;
	//����Ԫ�նȾ���Ӿֲ�����ϵת����ȫ������ϵ��ת�þ���,��ָ̬������
	//��Ҫ�����ͷ��ڴ�
	vector<Matrix2_6d*>	   Transto;
	vector<Matrix2_4d*>	   Transto2D;
	//ÿһ���˵ĳ���
	vector<double>		   link_length;

	~static_solver()
	{
		for (vector<Matrix2_6d*>::size_type i=0; i<Transto.size(); i++)
		{
			delete Transto[i];
			Transto[i] = NULL;
		}
	}
	void    readTrussData(const char* filename);
	void    save_results(const char* filename);
	void    Solve();
private:
	bool	SolveforDisplacement();
	bool	SolveforDisplacement2D();
	void	SolveforStress();
	void	GetStiffnessMat(const vec& nd1,const vec& nd2,double rad,Matrix6d& K,int id);
	void	GetStiffnessMat2D(const vec2& nd1,const vec2& nd2, double rad, Matrix4d& K,int id);
	void	InsertTriplet(vector<Tr>& tpl,int r,int c,const Matrix6d& K);
	void	InsertTriplet2D(vector<Tr>& tpl,int r,int c,const Matrix4d& K);
	void	SetBigNum(SparseMatrixType& K,int i);
	void	SetBigNum2D(SparseMatrixType& K,int i);
};

#endif
