/* Image-driven minimum-spanning-tree calcuation using Prim's algorithm.
 Average run-time should be of n*log(n) complexity.
 Uses heap data structure to speed-up finding the next lowest edge-weight.
 Edge-weights
*/

class Edge
{
public:
	double weight;
	int vert1;
	int vert2;
	Edge(double w=0,int v1=0,int v2=0);
	bool operator<(const Edge & b) const;
	void print();
};

Edge::Edge(double w,int v1,int v2){
	weight=w;
	vert1=v1;
	vert2=v2;
}

bool Edge::operator<(const Edge & b) const{
	return (this->weight>b.weight);
}

int newEdge(Edge edge1,Edge& edgeout,bool* vertices){
	bool new1=vertices[edge1.vert1];
	bool new2=vertices[edge1.vert2];
	int out1;
	if(new1^new2){
		// get the already connected vertex of the passed edge and return the other vertex
		if(new1){
			out1=edge1.vert2;
			edgeout=Edge(edge1.weight,edge1.vert1,edge1.vert2);
		}
		else {
			out1=edge1.vert1;
			edgeout=Edge(edge1.weight,edge1.vert2,edge1.vert1);
		}
	}
	else{
		out1=-1;
	}
	return out1;
}

float edgecost2weight(float val,float meanim){
    return exp(-val/meanim);
}

void primsGraph(float* im1,int* ordered,int* parents,float* edgemst,int step1,int m2,int n2,int o2){
	// takes image intensities, outputs ordered, parents and edgemst

	int m=m2/step1;
	int n=n2/step1;
	int o=o2/step1;

	int num_vertices=m*n*o; //num_patches
	int sz=num_vertices; //num_patches

	int len=m*n*o;
	timeval time1,time2;
	int num_neighbours=6;
	float* edgecost=new float[num_vertices*num_neighbours]; // size of count of edges
	int* index_neighbours=new int[num_vertices*num_neighbours]; // size of count of edges.
	for(int i=0;i<num_vertices*num_neighbours;i++){ // only edges between 6 patch neighbours
		edgecost[i]=0.0; // initialise cost to 0
		index_neighbours[i]=-1; //initialize
	}

	int dx[6]={-1,1,0,0,0,0};
	int dy[6]={0,0,-1,1,0,0};
	int dz[6]={0,0,0,0,-1,1};
	int xx,yy,zz,xx2,yy2,zz2;
	//calculate edge-weights based on SAD of groups of voxels (for each control-point)
	for(int k=0;k<o;k++){
		for(int j=0;j<n;j++){
			for(int i=0;i<m;i++){
				//iterate over all patches
				for(int nb=0;nb<num_neighbours;nb++){
					//iterate over all neighbours of a patch
					if((i+dy[nb])>=0&(i+dy[nb])<m&(j+dx[nb])>=0&(j+dx[nb])<n&(k+dz[nb])>=0&(k+dz[nb])<o){
						// only set edgecost and init val for index_neighbours if neighbour is within patch grid
						index_neighbours[i+j*m+k*m*n+nb*num_vertices]=i+dy[nb]+(j+dx[nb])*m+(k+dz[nb])*m*n;
						// store the coordinate of each neighbour of a patch (used to address neighbours of a patch later)
						//float randv=((float)rand()/float(RAND_MAX));
						//edgecost[i+j*m+k*m*n+nb*num_vertices]=randv;
						for(int k1=0;k1<step1;k1++){
							for(int j1=0;j1<step1;j1++){
								for(int i1=0;i1<step1;i1++){
									//iterate over all voxels in a patch
									xx=j*step1+j1;
									yy=i*step1+i1;
									zz=k*step1+k1; //voxel coordinates of a patch
									xx2=(j+dx[nb])*step1+j1;
									yy2=(i+dy[nb])*step1+i1;
									zz2=(k+dz[nb])*step1+k1; // voxel coordinates of the neighbour
									//calculate SAD of image intensities (not mind) and neighbour distance
									edgecost[i+j*m+k*m*n + nb*num_vertices] += fabs(im1[yy+xx*m2+zz*m2*n2]-im1[yy2+xx2*m2+zz2*m2*n2]);
									//edgecost of size patches * 6neighbours
								}
							}
						}
					}
				}
			}
		}
	}

    float meanim=0.0;
    for(int i=0;i<m2*n2*o2;i++){
        meanim+=im1[i];
    }
    meanim/=(float)(m2*n2*o2); // image mean intensity
	
	//normalize edgecosts
    float stdim=0.0;
    for(int i=0;i<m2*n2*o2;i++){
        stdim+=pow(im1[i]-meanim,2);
    }
    stdim=sqrt(stdim/(float)(m2*n2*o2));

	for(int i=0;i<sz*6;i++){
        edgecost[i]/=(float)pow(step1,3); // divide edge cost by voxel count of a patch
	}
    for(int i=0;i<sz*6;i++){
        edgecost[i]=-edgecost2weight(edgecost[i],2.0f*stdim); // edgecost = edgecost - exp(-edgecost/2*stdim)
	}

	float centrex=n/2;
	float centrey=m/2;
	float centrez=o/2; // patches center

	int root=m/2+n/2*m+o/2*m*n; // coordinate of the three root (middle of all patches)

	vector<Edge> priority;
	bool* vertices=new bool[num_vertices];
	int* level=new int[num_vertices];
	for(int i=0;i<num_vertices;i++){
		vertices[i]=false;
		parents[i]=-1;
	}

	//int root=0;
	level[root]=0;
	int last=root;
	vertices[root]=true; // visited patches?
	Edge edgeout=Edge(0.0,-1,-1);
	Edge minedge=Edge(0.0,-1,-1);
	float cost=0.0;
	gettimeofday(&time1, NULL);

	for(int i=0;i<num_vertices-1;i++){ //run n-1 times to have all vertices added
		//add edges of new vertex to priority queue
		// the final graph will contain num_patches-1 edges.

		for(int j=0;j<num_neighbours;j++){
			//visit every neigbour and push the edge to a priority queue
			int n=index_neighbours[last+j*num_vertices];
			if(n>=0){
				//if neighbour is valid (not outside of patch grid)
				priority.push_back(Edge(edgecost[last+j*num_vertices],last,n));
				push_heap(priority.begin(),priority.end()); //sorting of edges happens here.
			}
		}
		last=-1;
		//find valid edge with lowest weight (main step of Prim's algorithm)
		while(last==-1){
			minedge=priority.front();
			pop_heap(priority.begin(),priority.end());
			priority.pop_back();
			//clean heap and priority element together

			bool new1=vertices[minedge.vert1]; //is either vertex already part of MST?
			bool new2=vertices[minedge.vert2];
			last=newEdge(minedge,edgeout,vertices); //return next valid vertex or -1 if edge exists already
		}
		cost+=edgeout.weight;
		vertices[last]=true;
		level[edgeout.vert2]=level[edgeout.vert1]+1; //level of depth (depth(root)==0)
		parents[edgeout.vert2]=edgeout.vert1; // store parent vertex
	}

	//find correct ordering in constant time
	int maxlevel=0;
	for(int i=0;i<num_vertices;i++){
		if(level[i]>maxlevel)
			maxlevel=level[i];
	}
	maxlevel++;
	int* leveloffset=new int[maxlevel];
	int* levelcount=new int[maxlevel];
	for(int i=0;i<maxlevel;i++){
		leveloffset[i]=0;
		levelcount[i]=0;
	}

	for(int i=0;i<num_vertices;i++){
		if(level[i]<maxlevel-1)
			leveloffset[level[i]+1]++; //counting number of vertices in each level
	}
	for(int i=1;i<maxlevel;i++){
		leveloffset[i]+=leveloffset[i-1]; //vertex index at which a level "starts" e.g. if 40 vertices are in level 1,2,3 offset is 40 for level 4
	}
	for(int i=0;i<num_vertices;i++){
		int num=leveloffset[level[i]]+levelcount[level[i]];
		levelcount[level[i]]++; // counts number of vertices in a specific level
		ordered[num]=i; //ordered vertices (the later the vertex, the deeper the level
	}


	gettimeofday(&time2, NULL);
	double timeAll=time2.tv_sec+time2.tv_usec/1e6-(time1.tv_sec+time1.tv_usec/1e6);
    nth_element(levelcount,levelcount+maxlevel/2,levelcount+maxlevel);
	//printf("Prims algorithm with %d levels finished in %f secs.\nMaximum %d, minimum %d, mean %d, and median %d width of tree.\n",
          // maxlevel,timeAll,*max_element(levelcount,levelcount+maxlevel),*min_element(levelcount,levelcount+maxlevel),(int)(num_vertices/maxlevel),levelcount[maxlevel/2]);
	for(int i=0;i<sz;i++){
        edgemst[i]=0.0f;
    }
    for(int i=1;i<sz;i++){
		// for all-1 patches
		int ochild=ordered[i]; //get vertex
		int oparent=parents[ordered[i]]; // get its parent
        for(int nb=0;nb<num_neighbours;nb++){
            int z=ochild/(m*n);
			int x=(ochild-z*m*n)/m;
			int y=ochild-z*m*n-x*m; // get patch coordinates

            int index=y+dy[nb]+(x+dx[nb])*m+(z+dz[nb])*m*n;

            if(index==oparent){
                edgemst[ochild]=-edgecost[ochild+nb*sz];
				// message: substract edgecost of all connected children.
            }
        }
    }
	priority.clear();

	delete[] edgecost;
	delete[] index_neighbours;
	delete[] levelcount;
	delete[] leveloffset;
	delete[] vertices;
	delete[] level;
}

std::tuple<torch::Tensor, torch::Tensor, torch::Tensor> prims_mst_prims_graph(torch::Tensor pMind_img, torch::Tensor pGrid_divisor) {

	// Prepare input variables
    torch::Tensor mind_img_copy = pMind_img.clone();

	int m = mind_img_copy.size(0);
    int n = mind_img_copy.size(1);
    int o = mind_img_copy.size(2);

    float* mind_img = mind_img_copy.data_ptr<float>();
	int* grid_divisor = pGrid_divisor.data_ptr<int>();

	// Prepare output variables
    int* ordered = new int[m*n*o];
	int* parents = new int[m*n*o];
	float* edgemst = new float[m*n*o];

    primsGraph(mind_img, ordered, parents, edgemst, *grid_divisor, m, n, o);

	// Prepare lib output
	std::vector<int> ordered_vect{ordered, ordered+m*n*o};
	std::vector<int> parents_vect{parents, parents+m*n*o};
	std::vector<float> edgemst_vect{edgemst, edgemst+m*n*o};

	auto int_options = torch::TensorOptions().dtype(torch::kInt);
    auto float_options = torch::TensorOptions().dtype(torch::kFloat);
	int m_grid = (m/(*grid_divisor));
	int n_grid = (n/(*grid_divisor));
	int o_grid = (o/(*grid_divisor));
	return std::tuple<
            torch::Tensor,
            torch::Tensor,
            torch::Tensor>(
        torch::from_blob(ordered_vect.data(), {m_grid*n_grid*o_grid}, int_options).clone(),
        torch::from_blob(parents_vect.data(), {m_grid*n_grid*o_grid}, int_options).clone(),
        torch::from_blob(edgemst_vect.data(), {m_grid*n_grid*o_grid}, float_options).clone()
    );
}