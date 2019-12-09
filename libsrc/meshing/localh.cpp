#include <mystdlib.h>
#include "meshing.hpp"


namespace netgen
{

  GradingBox :: GradingBox (const double * ax1, const double * ax2)
  {
    h2 = 0.5 * (ax2[0] - ax1[0]);
    for (int i = 0; i < 3; i++)
      xmid[i] = 0.5 * (ax1[i] + ax2[i]);

    for (int i = 0; i < 8; i++)
      childs[i] = NULL;
    father = NULL;

    flags.cutboundary = 0;
    flags.isinner = 0;
    flags.oldcell = 0;
    flags.pinner = 0;

    hopt = 2 * h2;
  }


  BlockAllocator GradingBox :: ball(sizeof (GradingBox));

  void * GradingBox :: operator new(size_t)
  {
    return ball.Alloc();
  }

  void GradingBox :: operator delete (void * p)
  {
    ball.Free (p);
  }





  void GradingBox :: DeleteChilds()
  {
    for (int i = 0; i < 8; i++)
      if (childs[i])
	{
	  childs[i]->DeleteChilds();
	  delete childs[i];
	  childs[i] = NULL;
	}
  }
  

  LocalH :: LocalH (Point<3> pmin, Point<3> pmax, double agrading, int adimension)
    : dimension(adimension)
  {
    double x1[3], x2[3];
    double hmax;

    boundingbox = Box<3> (pmin, pmax);
    grading = agrading;

    // a small enlargement, non-regular points 
    double val = 0.0879;
    for (int i = 0; i < dimension; i++)
      {
	x1[i] = (1 + val * (i+1)) * pmin(i) - val * (i+1) * pmax(i);
	x2[i] = 1.1 * pmax(i) - 0.1 * pmin(i);
      }
    for (int i = dimension; i < 3; i++)
      x1[i] = x2[i] = 0;

    hmax = x2[0] - x1[0];
    for (int i = 1; i < dimension; i++)
      hmax = max2(x2[i]-x1[i], hmax);

    for (int i = 0; i < dimension; i++)
      x2[i] = x1[i] + hmax;

    root = new GradingBox (x1, x2);
    boxes.Append (root);
  }

  LocalH :: ~LocalH ()
  {
    root->DeleteChilds();
    delete root;
  }

  void LocalH :: Delete ()
  {
    root->DeleteChilds();
  }

  void LocalH :: SetH (Point<3> p, double h)
  {
    if (dimension == 2)
      {
        if (fabs (p(0) - root->xmid[0]) > root->h2 ||
            fabs (p(1) - root->xmid[1]) > root->h2)
          return;
        
        if (GetH(p) <= 1.2 * h) return;
        
        GradingBox * box = root;
        GradingBox * nbox = root;
        GradingBox * ngb;
        int childnr;
        double x1[3], x2[3];
        
        while (nbox)
          {
            box = nbox;
            childnr = 0;
            if (p(0) > box->xmid[0]) childnr += 1;
            if (p(1) > box->xmid[1]) childnr += 2;
            nbox = box->childs[childnr];
          };
        
        while (2 * box->h2 > h)
          {
            childnr = 0;
            if (p(0) > box->xmid[0]) childnr += 1;
            if (p(1) > box->xmid[1]) childnr += 2;
            
            double h2 = box->h2;
            if (childnr & 1)
              {
                x1[0] = box->xmid[0];
                x2[0] = x1[0]+h2;   // box->x2[0];
              }
            else
              {
                x2[0] = box->xmid[0];
                x1[0] = x2[0]-h2;   // box->x1[0];
              }
            
            if (childnr & 2)
              {
                x1[1] = box->xmid[1];
                x2[1] = x1[1]+h2;   // box->x2[1];
              }
            else
              {
                x2[1] = box->xmid[1];
                x1[1] = x2[1]-h2;   // box->x1[1];
              }
            x1[2] = x2[2] = 0;

            ngb = new GradingBox (x1, x2);
            box->childs[childnr] = ngb;
            ngb->father = box;
            
            boxes.Append (ngb);
            box = box->childs[childnr];
          }

        box->hopt = h;
        
        double hbox = 2 * box->h2;  // box->x2[0] - box->x1[0];
        double hnp = h + grading * hbox;
        
        Point<3> np;
        for (int i = 0; i < 2; i++)
          {
            np = p;
            np(i) = p(i) + hbox;
            SetH (np, hnp);
            
            np(i) = p(i) - hbox;
            SetH (np, hnp);
          }

      }

    else
      {
        if (fabs (p(0) - root->xmid[0]) > root->h2 ||
            fabs (p(1) - root->xmid[1]) > root->h2 ||
            fabs (p(2) - root->xmid[2]) > root->h2)
          return;
        
        if (GetH(p) <= 1.2 * h) return;
        
        GradingBox * box = root;
        GradingBox * nbox = root;
        GradingBox * ngb;
        int childnr;
        double x1[3], x2[3];
        
        while (nbox)
          {
            box = nbox;
            childnr = 0;
            if (p(0) > box->xmid[0]) childnr += 1;
            if (p(1) > box->xmid[1]) childnr += 2;
            if (p(2) > box->xmid[2]) childnr += 4;
            nbox = box->childs[childnr];
          };
        
        
        while (2 * box->h2 > h)
          {
            childnr = 0;
            if (p(0) > box->xmid[0]) childnr += 1;
            if (p(1) > box->xmid[1]) childnr += 2;
            if (p(2) > box->xmid[2]) childnr += 4;
            
            double h2 = box->h2;
            if (childnr & 1)
              {
                x1[0] = box->xmid[0];
                x2[0] = x1[0]+h2;   // box->x2[0];
              }
            else
              {
                x2[0] = box->xmid[0];
                x1[0] = x2[0]-h2;   // box->x1[0];
              }
            
            if (childnr & 2)
              {
                x1[1] = box->xmid[1];
                x2[1] = x1[1]+h2;   // box->x2[1];
              }
            else
              {
                x2[1] = box->xmid[1];
                x1[1] = x2[1]-h2;   // box->x1[1];
              }
            
            if (childnr & 4)
              {
                x1[2] = box->xmid[2];
                x2[2] = x1[2]+h2;  // box->x2[2];
              }
            else
              {
                x2[2] = box->xmid[2];
                x1[2] = x2[2]-h2;  // box->x1[2];
              }
            
            ngb = new GradingBox (x1, x2);
            box->childs[childnr] = ngb;
            ngb->father = box;
            
            boxes.Append (ngb);
            box = box->childs[childnr];
          }

        box->hopt = h;
        
        double hbox = 2 * box->h2;  // box->x2[0] - box->x1[0];
        double hnp = h + grading * hbox;
        
        Point<3> np;
        for (int i = 0; i < 3; i++)
          {
            np = p;
            np(i) = p(i) + hbox;
            SetH (np, hnp);
            
            np(i) = p(i) - hbox;
            SetH (np, hnp);
          }
      }
  }



  double LocalH :: GetH (Point<3> x) const
  {
    const GradingBox * box = root;
    if (dimension == 2)
      {
        while (1)
          {
            int childnr = 0;
            if (x(0) > box->xmid[0]) childnr += 1;
            if (x(1) > box->xmid[1]) childnr += 2;
            
            if (box->childs[childnr])
              box = box->childs[childnr];
            else
              return box->hopt;
          }
      }
    else
      {
        while (1)
          {
            int childnr = 0;
            if (x(0) > box->xmid[0]) childnr += 1;
            if (x(1) > box->xmid[1]) childnr += 2;
            if (x(2) > box->xmid[2]) childnr += 4;
            
            if (box->childs[childnr])
              box = box->childs[childnr];
            else
              return box->hopt;
          }
      }
      
  }


  /// minimal h in box (pmin, pmax)
  double LocalH :: GetMinH (Point<3> pmin, Point<3> pmax) const
  { 
    Point<3> pmin2, pmax2;
    for (int j = 0; j < 3; j++)
      if (pmin(j) < pmax(j))
	{ pmin2(j) = pmin(j); pmax2(j) = pmax(j); }
      else
	{ pmin2(j) = pmax(j); pmax2(j) = pmin(j); }

    return GetMinHRec (pmin2, pmax2, root); 
  }


  double LocalH :: GetMinHRec (const Point3d & pmin, const Point3d & pmax,
			       const GradingBox * box) const
  {
    if (dimension == 2)
      {
        double h2 = box->h2;
        if (pmax.X() < box->xmid[0]-h2 || pmin.X() > box->xmid[0]+h2 ||
            pmax.Y() < box->xmid[1]-h2 || pmin.Y() > box->xmid[1]+h2)
          return 1e8;
        
        double hmin = 2 * box->h2; // box->x2[0] - box->x1[0];
        
        for (int i = 0; i < 8; i++)
          if (box->childs[i])
            hmin = min2 (hmin, GetMinHRec (pmin, pmax, box->childs[i]));
        
        return hmin;
      }
    else
      {
        double h2 = box->h2;
        if (pmax.X() < box->xmid[0]-h2 || pmin.X() > box->xmid[0]+h2 ||
            pmax.Y() < box->xmid[1]-h2 || pmin.Y() > box->xmid[1]+h2 ||
            pmax.Z() < box->xmid[2]-h2 || pmin.Z() > box->xmid[2]+h2)
          return 1e8;
        
        double hmin = 2 * box->h2; // box->x2[0] - box->x1[0];
        
        for (int i = 0; i < 8; i++)
          if (box->childs[i])
            hmin = min2 (hmin, GetMinHRec (pmin, pmax, box->childs[i]));
        
        return hmin;
      }
  }









  void LocalH :: CutBoundaryRec (const Point3d & pmin, const Point3d & pmax,
				 GradingBox * box)
  {
    double h2 = box->h2;
    if (dimension == 2)
      {
        if (pmax.X() < box->xmid[0]-h2 || pmin.X() > box->xmid[0]+h2 ||
            pmax.Y() < box->xmid[1]-h2 || pmin.Y() > box->xmid[1]+h2)
          return;
      }
    else
      {
        if (pmax.X() < box->xmid[0]-h2 || pmin.X() > box->xmid[0]+h2 ||
            pmax.Y() < box->xmid[1]-h2 || pmin.Y() > box->xmid[1]+h2 ||
            pmax.Z() < box->xmid[2]-h2 || pmin.Z() > box->xmid[2]+h2)
          return;
      }

    box->flags.cutboundary = 1;
    for (int i = 0; i < 8; i++)
      if (box->childs[i])
	CutBoundaryRec (pmin, pmax, box->childs[i]);
  }



  void LocalH :: FindInnerBoxes (AdFront3 * adfront,
				 int (*testinner)(const Point3d & p1))
  {
    static int timer = NgProfiler::CreateTimer ("LocalH::FindInnerBoxes");
    NgProfiler::RegionTimer reg (timer);


    int nf = adfront->GetNF();

    for (int i = 0; i < boxes.Size(); i++)
      boxes[i] -> flags.isinner = 0;

    root->flags.isinner = 0;

    Point3d rpmid(root->xmid[0], root->xmid[1], root->xmid[2]);
    Vec3d rv(root->h2, root->h2, root->h2);
    Point3d rx2 = rpmid + rv;
    // Point3d rx1 = rpmid - rv;


    root->flags.pinner = !adfront->SameSide (rpmid, rx2);
    
    if (testinner)
      (*testout) << "inner = " << root->flags.pinner << " =?= " 
		 << testinner(Point3d(root->xmid[0], root->xmid[1], root->xmid[2])) << endl;

    NgArray<int> faceinds(nf);
    NgArray<Box3d> faceboxes(nf);

    for (int i = 1; i <= nf; i++)
      {
	faceinds.Elem(i) = i;
	adfront->GetFaceBoundingBox(i, faceboxes.Elem(i));
      }
  
    for (int i = 0; i < 8; i++)
      FindInnerBoxesRec2 (root->childs[i], adfront, faceboxes, faceinds, nf);
  }


  void LocalH :: 
  FindInnerBoxesRec2 (GradingBox * box,
		      class AdFront3 * adfront, 
		      NgArray<Box3d> & faceboxes,
		      NgArray<int> & faceinds, int nfinbox)
  {
    if (!box) return;
  
    GradingBox * father = box -> father;
  
    Point3d c(box->xmid[0], box->xmid[1], box->xmid[2]);
    Vec3d v(box->h2, box->h2, box->h2);
    Box3d boxc(c-v, c+v);

    Point3d fc(father->xmid[0], father->xmid[1], father->xmid[2]);
    Vec3d fv(father->h2, father->h2, father->h2);
    Box3d fboxc(fc-fv, fc+fv);

    Box3d boxcfc(c,fc);

    NgArrayMem<int, 100> faceused;
    NgArrayMem<int, 100> faceused2;
    NgArrayMem<int, 100> facenotused;

    /*
    faceused.SetSize(0);
    facenotused.SetSize(0);
    faceused2.SetSize(0);
    */

    for (int j = 1; j <= nfinbox; j++)
      {
	//      adfront->GetFaceBoundingBox (faceinds.Get(j), facebox);
	const Box3d & facebox = faceboxes.Get(faceinds.Get(j));
  
	if (boxc.Intersect (facebox))
	  faceused.Append(faceinds.Get(j));
	else
	  facenotused.Append(faceinds.Get(j));

	if (boxcfc.Intersect (facebox))
	  faceused2.Append (faceinds.Get(j));
      }
  
    for (int j = 1; j <= faceused.Size(); j++)
      faceinds.Elem(j) = faceused.Get(j);
    for (int j = 1; j <= facenotused.Size(); j++)
      faceinds.Elem(j+faceused.Size()) = facenotused.Get(j);

  
    if (!father->flags.cutboundary)
      {
	box->flags.isinner = father->flags.isinner;
	box->flags.pinner = father->flags.pinner;
      }
    else
      {
	Point3d cf(father->xmid[0], father->xmid[1], father->xmid[2]);
      
	if (father->flags.isinner)
	  box->flags.pinner = 1;
	else
	  {
	    if (adfront->SameSide (c, cf, &faceused2))
	      box->flags.pinner = father->flags.pinner;
	    else
	      box->flags.pinner = 1 - father->flags.pinner;
	  }
      
	if (box->flags.cutboundary)
	  box->flags.isinner = 0;
	else
	  box->flags.isinner = box->flags.pinner;
      }

    // cout << "faceused: " << faceused.Size() << ", " << faceused2.Size() << ", " << facenotused.Size() << endl;

    int nf = faceused.Size();
    for (int i = 0; i < 8; i++)
      FindInnerBoxesRec2 (box->childs[i], adfront, faceboxes, faceinds, nf);
  }



  void LocalH :: FindInnerBoxesRec ( int (*inner)(const Point3d & p),
				     GradingBox * box)
  {
    if (box->flags.cutboundary)
      {
	for (int i = 0; i < 8; i++)
	  if (box->childs[i])
	    FindInnerBoxesRec (inner, box->childs[i]);
      }
    else
      {
	if (inner (box->PMid()))
	  SetInnerBoxesRec (box);
      }
  }
















  void LocalH :: FindInnerBoxes (AdFront2 * adfront,
				 int (*testinner)(const Point<2> & p1))
  {
    static int timer = NgProfiler::CreateTimer ("LocalH::FindInnerBoxes 2d");
    NgProfiler::RegionTimer reg (timer);

    for (int i = 0; i < boxes.Size(); i++)
      boxes[i] -> flags.isinner = 0;

    root->flags.isinner = 0;

    Point<2> rpmid(root->xmid[0], root->xmid[1]);   // , root->xmid[2]);
    Vec<2> rv(root->h2, root->h2);
    Point<2> rx2 = rpmid + rv;
    // Point<2> rx1 = rpmid - rv;


    root->flags.pinner = !adfront->SameSide (rpmid, rx2);
  
    if (testinner)
      (*testout) << "inner = " << root->flags.pinner << " =?= "
		 << testinner(rpmid) << endl;


    int nf = adfront->GetNFL();
    NgArray<int> faceinds(nf);
    NgArray<Box<3> > faceboxes(nf);

    for (int i = 0; i < nf; i++)
      {
	faceinds[i] = i;
	// adfront->GetFaceBoundingBox(i, faceboxes.Elem(i));

	const FrontLine & line = adfront->GetLine(i);
	faceboxes[i].Set (adfront->GetPoint (line.L().I1()));
	faceboxes[i].Add (adfront->GetPoint (line.L().I2()));
      }
  
    for (int i = 0; i < 8; i++)
      FindInnerBoxesRec2 (root->childs[i], adfront, faceboxes, faceinds, nf);
  }


  void LocalH :: 
  FindInnerBoxesRec2 (GradingBox * box,
		      class AdFront2 * adfront, 
		      NgArray<Box<3> > & faceboxes,
		      NgArray<int> & faceinds, int nfinbox)
  {
    if (!box) return;
    
    GradingBox * father = box -> father;
  
    Point3d c(box->xmid[0], box->xmid[1], 0); // box->xmid[2]);
    Vec3d v(box->h2, box->h2, box->h2);
    Box3d boxc(c-v, c+v);

    Point3d fc(father->xmid[0], father->xmid[1], 0); // father->xmid[2]);
    Vec3d fv(father->h2, father->h2, father->h2);
    Box3d fboxc(fc-fv, fc+fv);
    Box3d boxcfc(c,fc);

    NgArrayMem<int, 100> faceused;
    NgArrayMem<int, 100> faceused2;
    NgArrayMem<int, 100> facenotused;

    for (int j = 0; j < nfinbox; j++)
      {
	//      adfront->GetFaceBoundingBox (faceinds.Get(j), facebox);
	const Box3d & facebox = faceboxes[faceinds[j]];
  
	if (boxc.Intersect (facebox))
	  faceused.Append(faceinds[j]);
	else
	  facenotused.Append(faceinds[j]);

	if (boxcfc.Intersect (facebox))
	  faceused2.Append (faceinds[j]);
      }
  
    for (int j = 0; j < faceused.Size(); j++)
      faceinds[j] = faceused[j];
    for (int j = 0; j < facenotused.Size(); j++)
      faceinds[j+faceused.Size()] = facenotused[j];
  
    if (!father->flags.cutboundary)
      {
	box->flags.isinner = father->flags.isinner;
	box->flags.pinner = father->flags.pinner;
      }
    else
      {
	Point3d cf(father->xmid[0], father->xmid[1], father->xmid[2]);
      
	if (father->flags.isinner)
          {
            box->flags.pinner = 1;
          }
	else
	  {
	    Point<2> c2d (c.X(), c.Y());
	    Point<2> cf2d (cf.X(), cf.Y());
            bool sameside = adfront->SameSide (c2d, cf2d, &faceused2);
            if (sameside)
	      box->flags.pinner = father->flags.pinner;
	    else
	      box->flags.pinner = 1 - father->flags.pinner;
	  }
      
	if (box->flags.cutboundary)
	  box->flags.isinner = 0;
	else
	  box->flags.isinner = box->flags.pinner;
      }

    // cout << "faceused: " << faceused.Size() << ", " << faceused2.Size() << ", " << facenotused.Size() << endl;

    int nf = faceused.Size();
    for (int i = 0; i < 8; i++)
      FindInnerBoxesRec2 (box->childs[i], adfront, faceboxes, faceinds, nf);
  }



  void LocalH :: FindInnerBoxesRec ( int (*inner)(const Point<2> & p),
				     GradingBox * box)
  {
    if (box->flags.cutboundary)
      {
	for (int i = 0; i < 8; i++)
	  if (box->childs[i])
	    FindInnerBoxesRec (inner, box->childs[i]);
      }
    else
      {
	Point<2> p2d(box->PMid()(0), box->PMid()(1)); 
	if (inner (p2d))
	  SetInnerBoxesRec (box);
      }
  }

















  void LocalH :: SetInnerBoxesRec (GradingBox * box)
  {
    box->flags.isinner = 1;
    for (int i = 0; i < 8; i++)
      if (box->childs[i])
	ClearFlagsRec (box->childs[i]);
  }

  void LocalH :: ClearFlagsRec (GradingBox * box)
  {
    box->flags.cutboundary = 0;
    box->flags.isinner = 0;
    for (int i = 0; i < 8; i++)
      if (box->childs[i])
	ClearFlagsRec (box->childs[i]);
  }


  void LocalH :: WidenRefinement ()
  {
    for (int i = 0; i < boxes.Size(); i++)
      {
	double h = boxes[i]->hopt;
	Point3d c = boxes[i]->PMid();
      
	for (int i1 = -1; i1 <= 1; i1++)
	  for (int i2 = -1; i2 <= 1; i2++)
	    for (int i3 = -1; i3 <= 1; i3++)
	      SetH (Point3d (c.X() + i1 * h, 
			     c.Y() + i2 * h,
			     c.Z() + i3 * h), 1.001 * h);     
      }
  }

  void LocalH :: GetInnerPoints (NgArray<Point<3> > & points)
  {
    if (dimension == 2)
      {
        for (int i = 0; i < boxes.Size(); i++)
          if (boxes[i] -> flags.isinner && boxes[i] -> HasChilds())
            points.Append ( boxes[i] -> PMid() );
      }
    else
      {
        for (int i = 0; i < boxes.Size(); i++)
          if (boxes[i] -> flags.isinner)
            points.Append ( boxes[i] -> PMid() );
      }
          
  }


  void LocalH :: GetOuterPoints (NgArray<Point<3> > & points)
  {
    for (int i = 0; i < boxes.Size(); i++)
      if (!boxes[i]->flags.isinner && !boxes[i]->flags.cutboundary)
	points.Append ( boxes[i] -> PMid());
  }


  void LocalH :: Convexify ()
  {
    ConvexifyRec (root);
  }

  void LocalH :: ConvexifyRec (GradingBox * box)
  {
    Point<3> center = box -> PMid();

    double size = 2 * box->h2; // box->x2[0] - box->x1[0];
    double dx = 0.6 * size;

    double maxh = box->hopt;
  
    for (int i = 0; i < 3; i++)
      {
	Point<3> hp = center;
	hp(i) += dx;
	maxh = max2 (maxh, GetH(hp));
	hp(i) = center(i)-dx;
	maxh = max2 (maxh, GetH(hp));
      }

    if (maxh < 0.95 * box->hopt)
      SetH (center, maxh);

    for (int i = 0; i < 8; i++)
      if (box->childs[i])
	ConvexifyRec (box->childs[i]);  
  }

  void LocalH :: PrintMemInfo (ostream & ost) const
  {
    ost << "LocalH: " << boxes.Size() << " boxes of " << sizeof(GradingBox)
	<< " bytes = " << boxes.Size()*sizeof(GradingBox) << " bytes" << endl;
  }
}
