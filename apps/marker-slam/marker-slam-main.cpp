#include <mrpt/system/os.h>
#include <mrpt/obs/CObservationImage.h>
#include <mrpt/detectors.h>
#include <mrpt/math/CMatrixTemplateNumeric.h>
#include <mrpt/math/CMatrixFixedNumeric.h>
#include <mrpt/gui/CDisplayWindow3D.h>
#include <mrpt/opengl/CGridPlaneXY.h>
#include <mrpt/opengl/CAxis.h>
#include <mrpt/opengl/CPointCloud.h>
#include <mrpt/opengl/stock_objects.h>
#include <mrpt/poses.h>

using namespace std;
using namespace mrpt::gui;
using namespace mrpt;
using namespace mrpt::utils;
using namespace mrpt::obs;
using namespace mrpt::detectors;
using namespace mrpt::poses;
using namespace mrpt::opengl;

#include <mrpt/examples_config.h>
#include <mrpt/mrpt_paths_config.h>
string myInitFile( MRPT_EXAMPLES_BASE_DIRECTORY + string("markerTest/MARKER_TEST.INI") );
string myCamFile(MRPT_SOURCE_BASE_DIRECTORY + string("/share/mrpt/datasets/markers/CAMERA_PARAM.INI"));

void addMarker(CDisplayWindow3D &win, const float markerSize, const vector_detectable_object &markers, const CPose3D &camPose, map<int, CPose3D> &markerMap)
{
	for(unsigned i = 0; i < markers.size(); ++i)
	{
		ASSERT_( IS_CLASS(markers[i],CDetectableMarker ) )
		CDetectableMarker::Ptr marker = std::dynamic_pointer_cast<CDetectableMarker>( markers[i] );
		// cout << "id" << i << "=" << marker->m_id << endl;
		// cout << "corners" << i << "=[" ;
		//for( unsigned int j = 0; j < marker->corners.size() - 1; ++j)
			// cout << marker->corners[j].first << ", " << marker->corners[j].second << ", ";
		// cout << marker->corners[marker->corners.size() - 1].first << ", "
		// 	<< marker->corners[marker->corners.size() - 1].second << "]" << endl;
		// cout << "pose" << i << "=["
		// 	 << marker->m_pose.m_coords[0] << ", "
		// 	 << marker->m_pose.m_coords[1] << ", "
		// 	 << marker->m_pose.m_coords[2] << ", "
		// 	 << marker->m_pose.m_rotvec[0] << ", "
		// 	 << marker->m_pose.m_rotvec[1] << ", "
		// 	 << marker->m_pose.m_rotvec[2] << "]" << endl;
		map<int, CPose3D>::const_iterator it = markerMap.find(marker->m_id);

		if(it == markerMap.end()){
			// cout << camPose << endl;
			// cout << CPose3D(marker->m_pose) << endl;
			// cout << camPose + CPose3D(marker->m_pose) << endl;
			markerMap.insert(make_pair(marker->m_id, camPose + CPose3D(marker->m_pose)));
			// cout << CPose3D(marker->m_pose) << endl;
			// cout << camPose << endl;
			// cout << camPose + CPose3D(marker->m_pose) << endl;
		}
		else
			 cout << "Final " << marker->m_id << " " << camPose + CPose3D(marker->m_pose) << endl;
		// cout << "Inserted " << (marker->m_id) << " " << CPose3D(marker->m_pose) + camPose << endl;
		COpenGLScene::Ptr &theScene = win.get3DSceneAndLock();
		opengl::CGridPlaneXY::Ptr obj = opengl::CGridPlaneXY::Create(-1,1,-1,1,0,1, 0.4);
		obj->setPose(camPose +CPose3D(marker->m_pose));
		obj->setColor(0.8,0.8,0.8);
		obj->setScale(markerSize/2);
		theScene->insert(obj);
		win.unlockAccess3DScene();
			// Update window:
		win.forceRepaint();
	}
}

void addFrame(CDisplayWindow3D &win, CPose3D &pose)
{
	COpenGLScene::Ptr &theScene = win.get3DSceneAndLock();
	opengl::CSetOfObjects::Ptr obj = opengl::stock_objects::CornerXYZ();
	obj->setPose(pose);
	theScene->insert(obj);
	win.unlockAccess3DScene();
	// Update window:
	win.forceRepaint();
}

bool getFramePose(const map<int, CPose3D> &markerMap, const vector_detectable_object &markers, CPose3D &camPose)
{
	Eigen::Vector3f mu_x = {0, 0, 0};
	Eigen::Vector3f mu_y = {0, 0, 0};
	Eigen::Matrix3f covar = Eigen::Matrix3f::Zero();;
	// cout << covar << endl;
	vector< mrpt::math::CArrayDouble<3> > vecX;
	vector< mrpt::math::CArrayDouble<3> > vecY;
	int n = 0;
	for ( unsigned int i = 0; i < markers.size(); i++ )
	{
		ASSERT_( IS_CLASS(markers[i],CDetectableMarker ) )
		CDetectableMarker::Ptr obj = std::dynamic_pointer_cast<CDetectableMarker>(markers[i]);
		map<int, CPose3D>::const_iterator it = markerMap.find(obj->m_id);
		//cout << "Finding" << (obj->m_id) << endl;
		if(it != markerMap.end()){
			//camPose = it->second - CPose3D(obj->m_pose);
			mrpt::math::CMatrixDouble44 hom;
			mrpt::math::CArrayDouble<4> markerPoint;
			mrpt::math::CArrayDouble<4> homog_point;
			mrpt::math::CArrayDouble<3> point3D;

			float testx=0;

			markerPoint[0] = 0.123/2;
			markerPoint[1] = 0.123/2;
			markerPoint[2] = 0;
			markerPoint[3] = 1;

			it->second.getHomogeneousMatrix(hom);
			homog_point = hom*markerPoint;
			point3D(0) = homog_point(0)/homog_point(3);
			point3D(1) = homog_point(1)/homog_point(3);
			point3D(2) = homog_point(2)/homog_point(3);
			
			vecX.push_back(point3D);
			testx += point3D(0);

			obj->m_pose.getHomogeneousMatrix(hom);
			homog_point = hom*markerPoint;
			point3D(0) = homog_point(0)/homog_point(3);
			point3D(1) = homog_point(1)/homog_point(3);
			point3D(2) = homog_point(2)/homog_point(3);
			
			vecY.push_back(point3D);

			markerPoint[0] = -0.123/2;
			markerPoint[1] = 0.123/2;
			markerPoint[2] = 0;
			markerPoint[3] = 1;

			it->second.getHomogeneousMatrix(hom);
			homog_point = hom*markerPoint;
			point3D(0) = homog_point(0)/homog_point(3);
			point3D(1) = homog_point(1)/homog_point(3);
			point3D(2) = homog_point(2)/homog_point(3);
			
			vecX.push_back(point3D);
			testx += point3D(0);

			obj->m_pose.getHomogeneousMatrix(hom);
			homog_point = hom*markerPoint;
			point3D(0) = homog_point(0)/homog_point(3);
			point3D(1) = homog_point(1)/homog_point(3);
			point3D(2) = homog_point(2)/homog_point(3);
			
			vecY.push_back(point3D);

			markerPoint[0] = 0.123/2;
			markerPoint[1] = -0.123/2;
			markerPoint[2] = 0;
			markerPoint[3] = 1;

			it->second.getHomogeneousMatrix(hom);
			homog_point = hom*markerPoint;
			point3D(0) = homog_point(0)/homog_point(3);
			point3D(1) = homog_point(1)/homog_point(3);
			point3D(2) = homog_point(2)/homog_point(3);
			
			vecX.push_back(point3D);
			testx += point3D(0);

			obj->m_pose.getHomogeneousMatrix(hom);
			homog_point = hom*markerPoint;
			point3D(0) = homog_point(0)/homog_point(3);
			point3D(1) = homog_point(1)/homog_point(3);
			point3D(2) = homog_point(2)/homog_point(3);
			
			vecY.push_back(point3D);

			markerPoint[0] = -0.123/2;
			markerPoint[1] = -0.123/2;
			markerPoint[2] = 0;
			markerPoint[3] = 1;

			it->second.getHomogeneousMatrix(hom);
			homog_point = hom*markerPoint;
			point3D(0) = homog_point(0)/homog_point(3);
			point3D(1) = homog_point(1)/homog_point(3);
			point3D(2) = homog_point(2)/homog_point(3);
			
			vecX.push_back(point3D);
			testx += point3D(0);

			obj->m_pose.getHomogeneousMatrix(hom);
			homog_point = hom*markerPoint;
			point3D(0) = homog_point(0)/homog_point(3);
			point3D(1) = homog_point(1)/homog_point(3);
			point3D(2) = homog_point(2)/homog_point(3);
			
			vecY.push_back(point3D);
			++n;
			//cout << "Campose " << camPose << endl;
			cout << "in map " << it->second << endl;
			cout << testx/4 << endl;
			//cout << camPose + CPose3D(obj->m_pose);
			// CPose3D temp(obj->m_pose);
			// temp.inverse();
			// cout << CPose3D(obj->m_pose) + temp << endl;
			//return true;
		}
	}
	if(n>0){

		for(int i = 0; i < vecY.size(); ++i){
			mu_x(0) += vecX[i][0];
			mu_x(1) += vecX[i][1];
			mu_x(2) += vecX[i][2];
			mu_y(0) += vecY[i][0];
			mu_y(1) += vecY[i][1];
			mu_y(2) += vecY[i][2];
		}

		mu_x /= (4*n);
		mu_y /= (4*n);
		Eigen::Vector3f del_y;
		Eigen::Vector3f del_x;

		for(int i = 0; i < vecY.size(); ++i){
			del_y(0) = vecY[i](0);
			del_y(1) = vecY[i](1);
			del_y(2) = vecY[i](2);
			del_x(0) = vecX[i](0);
			del_x(1) = vecX[i](1);
			del_x(2) = vecX[i](2);
			covar += (del_y-mu_y)*((del_x-mu_x).transpose());
		}

		covar /= 4*n;
		cout << "covar" << endl << covar << endl;
		Eigen::JacobiSVD<Eigen::Matrix3f> svd(covar,Eigen::ComputeThinU | Eigen::ComputeThinV);
		Eigen::Vector3f vs = svd.singularValues();
		Eigen::Matrix3f D = Eigen::Matrix3f::Zero();
		D(0,0)=vs[0];
		D(1,1)=vs[1];
		D(2,2)=vs[2];
		cout << endl;
		Eigen::Matrix3f S = Eigen::Matrix3f::Identity();
		int uvt = round(svd.matrixU().determinant()*svd.matrixV().determinant());
		cout << "uvt " << uvt << endl;
		if(uvt == -1){
			cout << "negative" << endl;
			S(2, 2) = -1;
		}
		Eigen::Matrix3f Rot = svd.matrixU()*S*svd.matrixV().transpose();
		cout << "Rotation:" << endl << Rot << endl;
		cout << Rot.determinant() << endl;
		Eigen::Vector3f translation = mu_y - Rot*mu_x;
		cout << "Trans:" << endl << translation << endl; 
		//cout << (svd.matrixU() * D )* svd.matrixV().transpose();
		//cout << "U:" << endl << svd.matrixU() << endl << "D:" << endl << D << endl << "Vt:" << endl << svd.matrixV() << endl;
		// cout << svd.matrixU().determinant() << endl;
		camPose.m_coords(0) = translation(0);
		camPose.m_coords(1) = translation(1);
		camPose.m_coords(2) = translation(2);

		mrpt::math::CMatrixDouble33 rot;
		for (int i = 0; i < 3; ++i)
		{
			for (int j = 0; j < 3; ++j)
			rot(i, j) = Rot(i, j);
		}
		camPose.setRotationMatrix(rot);
		camPose.inverse();
		cout << "new campose: " << endl << camPose << endl;
		// camPose.setFromValues(camPose[0], camPose[1], camPose[2], camPose[3], camPose[4] - 1.5707963267948966, camPose[5]);
		return true;
	}
	else
		return false;
}

int main(int argc, char** argv){
	map<int, CPose3D> markerMap;
	CObservationImage::Ptr obsImg = CObservationImage::Ptr(new CObservationImage);
	CImage img;
	CMarkerDetection<CArucoDetectionPolicy> det;
	CDisplayWindow3D win("Example of 3D Scene Visualization - MRPT",640,480);

	//Load Camera Param
	CStringList camLst;	
	CConfigFileMemory camCfg; 
	camLst.loadFromFile(myCamFile);
	camCfg.setContent(camLst);
	obsImg->cameraParams.loadFromConfigFile(camCfg, string("CameraParams"));

	//Load Detection configurations
	CStringList lst;	
	CConfigFileMemory cfg; 
	lst.loadFromFile(myInitFile);
	cfg.setContent(lst);
	det.init(cfg);

	string imgName;
	if(argc < 2){
		cout << "Provide Filename while running" << endl;
		return 0;
	}

	for(int num = 1; num < argc; ++num){
		imgName = string(argv[num]); // get filename

		if (!img.loadFromFile(imgName))
			throw std::runtime_error("Can't load demo image!");
		obsImg->image = img;
		vector_detectable_object detected;
		det.detectObjects( obsImg, detected );

		// COpenGLScene::Ptr &theScene = win.get3DSceneAndLock();

		// {
		// 	theScene->insert( opengl::stock_objects::CornerXYZ());
		// }

		//win.unlockAccess3DScene();

		if ( detected.size() > 0 )
		{	
			CPose3D camPose;
			if(markerMap.empty())
			{
				addMarker(win, 0.123, detected, camPose, markerMap);
			}
			else
			{
				if(getFramePose(markerMap, detected, camPose)){
					addMarker(win, 0.123, detected, camPose, markerMap);
				}
				else{
					cout << "No common marker" << endl;
					break;
				}
			}
			addFrame(win, camPose);
		}
	}
	bool end = false;
	while (!end && win.isOpen() )
	{
		std::this_thread::sleep_for(20ms);

		if (mrpt::system::os::kbhit()) end = true;
		if (win.keyHit())
		{
			mrptKeyModifier kmods;
			int key = win.getPushedKey(&kmods);
			printf("Key pushed: %c (%i) - modifiers: 0x%04X\n",char(key),key,kmods);

			if (key==MRPTK_ESCAPE) end = true;

			if (key==MRPTK_RIGHT) win.setCameraAzimuthDeg( win.getCameraAzimuthDeg() + 5 );
			if (key==MRPTK_LEFT)  win.setCameraAzimuthDeg( win.getCameraAzimuthDeg() - 5 );
		}
	}

}