#include "FiniteElemSolver.h"
#include <cstdio>

void static_solver::readTrussData(const char* filename)
{
	FILE *file;
	if((file=fopen(filename,"r"))==NULL)
	{
		printf("file not exists,please check the path of the file");
		return;
	}
	// vec named str
	char str[80];
	// %s: space, tab, newline
	while (fscanf(file,"%s",str) == 1)
	{
		if (strncmp(str,"elasticModulus",14) == 0)
		{
			float tmpex;
			fscanf(file,"%f",&tmpex);
			ex = tmpex;
		}
		else if (strncmp(str,"nodes",5) == 0)
		{
			int node_count;
			fscanf(file,"%d",&node_count);
			for (int i=0; i<node_count; i++)
			{
				float x,y;
				fscanf(file,"%f %f",&x,&y);
				nd2.push_back(vec2(x,y));
			}
		}
		// elements I suppose
		else if (strncmp(str,"units",5) == 0)
		{
			int unit_count;
			fscanf(file,"%d",&unit_count);
			un.resize(unit_count);
			link_rad.resize(unit_count);
			for (int i=0; i<unit_count; i++)
			{
				int point1,point2; float rad;
				fscanf(file,"%d %d %f",&point1,&point2,&rad);
				un[i].first = point1;
				un[i].second = point2;
				link_rad[i] = rad;
			}
		}
		else if (strncmp(str,"force",5) == 0)
		{
			int force_count;
			fscanf(file,"%d",&force_count);
			for (int i=0; i<force_count; i++)
			{
				int f_index; float x,y;
				fscanf(file,"%d %f %f",&f_index,&x,&y);
				ext_f2d.push_back(pair<int,vec2>(f_index,vec2(x,y)));
			}
		}
		else if (strncmp(str,"constraint",10) == 0)
		{
			int cons_count;
			fscanf(file,"%d",&cons_count);
			for (int i=0; i<cons_count; i++)
			{
				int cons_index;
				fscanf(file,"%d",&cons_index);
				constraint.push_back(cons_index);
			}
		}
		else
		{
			printf("wrong file\n");
		}
	}
	fclose(file);
}

void static_solver::Solve()
{
	if (SolveforDisplacement2D())
	{
		printf("success\n");
		SolveforStress();
	}
	else
	{
		printf("there are some errors in the truss structure\n");
		return;
	}
}

bool static_solver::SolveforDisplacement2D()
{
	int unit_count = un.size();
	int nd_count = nd2.size();
	int dimf_count = 2 * nd_count;
	vector<Tr> tripletList;
	tripletList.reserve(16*unit_count);
	Transto2D.resize(unit_count);
	link_length.resize(unit_count);

	for (int i=0; i<unit_count; i++)
	{
		//p is first node of the strut , q is the second 
		int p = un[i].first, q = un[i].second;
		Matrix4d uk;
		GetStiffnessMat2D(nd2[p],nd2[q],link_rad[i],uk,i);
		InsertTriplet2D(tripletList,p,q,uk);
	}

	SparseMatrixType Kw(2*nd_count,2*nd_count);
	Kw.setFromTriplets(tripletList.begin(),tripletList.end());
	std::cout << "setFromTriplets" << Kw << "\n" << std::endl;

	Eigen::VectorXd Fr(dimf_count);
	Fr.setZero();

	//set external force 
	for (size_t i=0; i<ext_f2d.size(); i++)
	{
		for (int j=0; j<2; j++)
			Fr(2*ext_f2d[i].first+j) = ext_f2d[i].second[j];
	}

	// std::cout << Kw << std::endl;
	//set constraint 
	for (size_t i=0; i< constraint.size(); i++)
	{
		for (int j=0; j<2; j++)
			Fr(2*constraint[i]+j) = 0;

		SetBigNum2D(Kw,constraint[i]);
	}

	std::cout << "global matrix" << Kw << "\n" << std::endl;
	
	Eigen::SimplicialCholesky<SparseMatrixType> K_Solver;
	K_Solver.compute(Kw);
  	if (K_Solver.info() != Eigen::Success)
	{
		std::cout << K_Solver.info() << std::endl;
		return false;
	}         
	
	du = K_Solver.solve(Fr);
	return true;
}

bool static_solver::SolveforDisplacement()
{
	int unit_count = un.size();
	int nd_count = nd.size();
	int dimf_count = 3 * nd_count;
	vector<Tr> tripletList;
	tripletList.reserve(36*unit_count);
	Transto.resize(unit_count);
	link_length.resize(unit_count);

	for (int i=0; i<unit_count; i++)
	{
		//p is first node of the strut , q is the second 
		int p = un[i].first, q = un[i].second;
		Matrix6d uk;
		GetStiffnessMat(nd[p],nd[q],link_rad[i],uk,i);
		InsertTriplet(tripletList,p,q,uk);
	}

	SparseMatrixType Kw(3*nd_count,3*nd_count);
	Kw.setFromTriplets(tripletList.begin(),tripletList.end());

	Eigen::VectorXd Fr(dimf_count);
	Fr.setZero();

	//set external force 
	for (size_t i=0; i<ext_f.size(); i++)
	{
		for (int j=0; j<3; j++)
			Fr(3*ext_f[i].first+j) = ext_f[i].second[j];
	}

	// std::cout << Kw << std::endl;
	//set constraint 
	for (size_t i=0; i< constraint.size(); i++)
	{
		for (int j=0; j<3; j++)
			Fr(3*constraint[i]+j) = 0;

		SetBigNum(Kw,constraint[i]);
	}

	
	Eigen::SimplicialCholesky<SparseMatrixType> K_Solver;
	std::cout << Kw << std::endl;
	K_Solver.compute(Kw);
  	if (K_Solver.info() != Eigen::Success)
	{
		std::cout << K_Solver.info() << std::endl;
		return false;
	}         
	
	du = K_Solver.solve(Fr);
	return true;
}

void static_solver::SetBigNum2D(SparseMatrixType& K,int i)
{
	i *= 2;
	for (int j=0; j<2; j++)
	{
		for (int k=0; k<2; k++)
		{
			float ef = K.coeff(i+j,i+k);
			if (ef <= BIGNUMERIC) {
			K.coeffRef(i+j,i+k) = BIGNUMERIC * ef;
			}
		}
	}
}

void static_solver::SetBigNum(SparseMatrixType& K,int i)
{
	i *= 3;
	for (int j=0; j<3; j++)
	{
		for (int k=0; k<3; k++)
		{
			float ef = K.coeff(i+j,i+k);
			if (ef <= BIGNUMERIC) {
			K.coeffRef(i+j,i+k) = BIGNUMERIC * ef;
			}
		}
	}
}

void static_solver::InsertTriplet2D(vector<Tr>& tpl,int r,int c,const Matrix4d& K)
{
	r *= 2; c *= 2;

	for (int i=0,k=0; i<2; i++,k++)
		for (int j=0,l=0; j<2; j++,l++)
			tpl.push_back(Tr(r+k,r+l,K(i,j)));

	for (int i=0,k=0; i<2; i++,k++)
		for (int j=2,l=0; j<4; j++,l++)
			tpl.push_back(Tr(r+k,c+l,K(i,j)));

	for (int i=2,k=0; i<4; i++,k++)
		for (int j=0,l=0; j<2; j++,l++)
			tpl.push_back(Tr(c+k,r+l,K(i,j)));

	for (int i=2,k=0; i<4; i++,k++)
		for (int j=2,l=0; j<4; j++,l++)
			tpl.push_back(Tr(c+k,c+l,K(i,j)));
}

//     r      c
//   * * *  * * *
//r  * * *  * * *
//   * * *  * * * 
//
//   * * *  * * * 
//c  * * *  * * *
//   * * *  * * * 
void static_solver::InsertTriplet(vector<Tr>& tpl,int r,int c,const Matrix6d& K)
{
	r *= 3; c *= 3;

	for (int i=0,k=0; i<3; i++,k++)
		for (int j=0,l=0; j<3; j++,l++)
			tpl.push_back(Tr(r+k,r+l,K(i,j)));

	for (int i=0,k=0; i<3; i++,k++)
		for (int j=3,l=0; j<6; j++,l++)
			tpl.push_back(Tr(r+k,c+l,K(i,j)));

	for (int i=3,k=0; i<6; i++,k++)
		for (int j=0,l=0; j<3; j++,l++)
			tpl.push_back(Tr(c+k,r+l,K(i,j)));

	for (int i=3,k=0; i<6; i++,k++)
		for (int j=3,l=0; j<6; j++,l++)
			tpl.push_back(Tr(c+k,c+l,K(i,j)));
}

inline void static_solver::GetStiffnessMat(const vec& nd1,const vec& nd2,
										  double rad, Matrix6d& K,int id)
{
	assert(rad > 0);
	const double pi = 3.14159265359;
	double ld = len(nd2 - nd1);
	assert(ld > 0);
	double area = pi * rad * rad;
	double coefficient = ex * area / ld;

	//cos(x,x~) = (x2 - x1) / l
	double md = (nd2[0] - nd1[0]) / ld;
	//cos(x,y~) = (y2 - y1) / l
	double nd = (nd2[1] - nd1[1]) / ld;
	//cos(x,z~) = (z2 - z1) / l
	double rd = (nd2[2] - nd1[2]) / ld;

	Eigen::Matrix<double,2,6>* Te = new Eigen::Matrix<double,2,6>;
	*Te << md, nd, rd, 0, 0, 0,
		   0, 0, 0, md, nd, rd;

	Eigen::Matrix2d Ke;
	Ke << 1, -1, -1, 1;

	K = coefficient * ((*Te).transpose() * Ke * (*Te));
std::cout << K << std::endl;
	for (int i=2;i<6;i+=3)
	{
		for (int j=2;j<6;j+=3)
		{
			K(i,j) = BIGNUMERIC;
		}
	}

std::cout << K << std::endl;


	Transto[id] = Te;
	link_length[id] = ld;
}

inline void static_solver::GetStiffnessMat2D(const vec2& nd1,const vec2& nd2, double rad, Matrix4d& K,int id)
{
	assert(rad > 0);
	const double pi = 3.14159265359;
	double ld = len(nd2 - nd1);
	assert(ld > 0);
	double area = pi * rad * rad;
	double coefficient = ex * area / ld;

	//cos(x,x~) = (x2 - x1) / l
	double md = (nd2[0] - nd1[0]) / ld;
	//cos(x,y~) = (y2 - y1) / l
	double nd = (nd2[1] - nd1[1]) / ld;

	Eigen::Matrix<double,2,4>* Te = new Eigen::Matrix<double,2,4>;
	*Te << md, nd, 0, 0,
				0, 0, md, nd;

	Eigen::Matrix2d Ke;
	Ke << 1, -1, -1, 1;

	K = coefficient * ((*Te).transpose() * Ke * (*Te));
	std::cout << K << "\n" << std::endl;

	Transto2D[id] = Te;
	link_length[id] = ld;
}

void static_solver::SolveforStress()
{
	assert(!un.empty());
	if (un.empty())
		return;

	stress.clear();
	int un_count = un.size();
	stress.resize(un_count);
	Eigen::Matrix<double,1,2> unit;
	unit << -1.0,1.0;
	double coeffs = 0.0;
	int nd1,nd2;

	for (int i=0; i<un_count; i++)
	{
		coeffs = ex / link_length[i];
		nd1 = 2 * un[i].first; 
		nd2 = 2 * un[i].second;
		Eigen::Matrix<double,4,1> dq;
		dq << du[nd1],du[nd1+1],
			  du[nd2],du[nd2+1];

		stress[i] = coeffs * unit * (*Transto2D[i]) * dq;
	}
}

void static_solver::save_results(const char* filename)
{
	FILE *fp;
	char triname[256];

	strcpy(triname,filename);

	if ((fp = fopen(triname,"w")) == NULL)
	{
		printf("error");
		return;
	}

	fprintf(fp,"results data\n");
	fprintf(fp,"Nodes Displacement\n");
	int nd_count = du.size() / 2;
	for (int i=0; i<nd_count; i++)
	{
		int n_index = i*2;
		fprintf(fp,"Node%d:%.6f,%.6f\n",i,du[n_index],du[n_index+1]);
	}

	fprintf(fp,"Element Unit Stress\n");
	for (size_t i=0; i<stress.size(); i++)
	{
		fprintf(fp,"Unit%lu:%.3f\n",i,static_cast<float>(stress[i]));
	}
	fclose(fp);
}
