#include "stdafx.h"

#include "NifCmd.h"
#include "nifutils.h"
#include "obj/NiTriBasedGeom.h"
#include "obj/NiTriBasedGeomData.h"
#include "obj/NiTriShape.h"
#include "obj/NiTriStrips.h"
#include "obj/bhkRigidBody.h"
#include "Inertia.h"

#include <stdio.h>
using namespace std;


struct NiNodeNameEquivalence : public NumericStringEquivalence
{
	bool operator()(const NiNodeRef& n1, const NiNodeRef& n2) const { 
		return NumericStringEquivalence::operator()(n1->GetName(), n2->GetName());
	}
};

// sprintf for std::string without having to worry about buffer size.
static std::string FormatString(const TCHAR* format,...)
{
	TCHAR buffer[512];
	std::string text;
	va_list args;
	va_start(args, format);
	int nChars = _vsntprintf(buffer, _countof(buffer), format, args);
	if (nChars != -1) {
		text = buffer;
	} else {
		size_t Size = _vsctprintf(format, args);
		TCHAR* pbuf = (TCHAR*)_alloca(Size);
		nChars = _vsntprintf(pbuf, Size, format, args);
		text = pbuf;
	}
	va_end(args);
	return text;
}

static Matrix44 transRotate(const Matrix44& tm){
   static const Matrix44 r90 (1.0f,0.0f,0.0f,0.0f, 0.0f,0.0f,-1.0f,0.0f, 0.0f,1.0f,0.0f,0.0f, 0.0f,0.0f,0.0f,1.0f);
   Matrix44 m = (tm * r90);
   Vector3 t = m.GetTranslation();
   m = m.Transpose() * r90;
   m[3] = Float4(0.0f, 0.0f, 0.0f, 1.0f);
   m[0][3] = t[0];
   m[1][3] = t[1];
   m[2][3] = t[2];
   m[3][3] = 1.0f;
   return m.Transpose();
}

class FigWriter
{
	FILE *hOut;
public:
	FigWriter() {
		hOut = stdout;
	}
	FigWriter(FILE *hFile) {
		hOut = hFile;
	}

	void writeLine() {
		fwrite("\n", 1, 1, hOut);
	}

	void writeLine(const char *str) {
		fwrite(str, 1, strlen(str), hOut);
		fwrite("\n", 1, 1, hOut);
	}

	void writeLine(int value) {
		_ftprintf(hOut, "%d", value);
		fwrite("\n", 1, 1, hOut);
	}

	void writeLine(float value) {
		_ftprintf(hOut, "%f", (double)value);
		fwrite("\n", 1, 1, hOut);
	}

	void writeLine(double value) {
		_ftprintf(hOut, "%f", value);
		fwrite("\n", 1, 1, hOut);
	}

	void write(const char *str) {
		fwrite(str, 1, strlen(str), hOut);
	}

	void writeLine(const Matrix44& m) {
      formatLine( "%f %f %f %f", m[0][0], m[0][1], m[0][2], m[0][3]);
      formatLine( "%f %f %f %f", m[1][0], m[1][1], m[1][2], m[1][3]);
      formatLine( "%f %f %f %f", m[2][0], m[2][1], m[2][2], m[2][3]);
      formatLine( "%f %f %f %f", m[3][0], m[3][1], m[3][2], m[3][3]);
   }	
	void writeLine(const Vector4& v) {
		// weird transform
		formatLine( "%f %f %f %f", v[0], v[1], v[2], v[3]);
	}
	void writeLine(const Quaternion& q) {
		// weird transform
		formatLine( "%f\n%f\n%f\n%f", q.x, q.y, q.z, q.w);
	}
	
	void formatLine(const char *format, ... ) {
		va_list args;
		va_start(args, format);
		_vftprintf(hOut, format, args);
		writeLine();
		va_end(args);
	}

};



static void ConvertSkeletonToFig(vector<NiNodeRef> nodes, FILE* hOutFile, int userver)
{
	try 
	{
		FigWriter out(hOutFile);

		vector<NiNodeRef> bipedRoots = SelectNodesByName(nodes, "Bip??");
		std::stable_sort(bipedRoots.begin(), bipedRoots.end(), NiNodeNameEquivalence());
		for (vector<NiNodeRef>::iterator bipedItr = bipedRoots.begin(); bipedItr != bipedRoots.end(); ++bipedItr)
		{
			string bipname = (*bipedItr)->GetName();
			const char *bip = bipname.c_str();
			string match = bipname + "*";
			vector<NiNodeRef> bipedNodes = SelectNodesByName(nodes, match.c_str());

			float bipedHeight = 131.90f;
			float bipedAngle = 90.0f;
			float bipedAnkleAttach = 0.2f;
			bool bipedTrianglePelvis = true;
			float angle = TORAD(bipedAngle);
			BOOL arms = (CountNodesByName(bipedNodes, FormatString("%s L UpperArm", bip)) > 0) ? TRUE : FALSE;
			BOOL triPelvis = bipedTrianglePelvis ? TRUE : FALSE;
			int nnecklinks=CountNodesByName(bipedNodes, FormatString("%s Neck*", bip));
			int nspinelinks=CountNodesByName(bipedNodes, FormatString("%s Spine*", bip));
			int nleglinks = 3 + CountNodesByName(bipedNodes, FormatString("%s L HorseLink", bip));
			int ntaillinks = CountNodesByName(bipedNodes, FormatString("%s Tail*", bip));
			int npony1links = CountNodesByName(bipedNodes, FormatString("%s Ponytail1*", bip));
			int npony2links = CountNodesByName(bipedNodes, FormatString("%s Ponytail2*", bip));
			int numfingers = CountNodesByName(bipedNodes, FormatString("%s L Finger?", bip));
			int nfinglinks = CountNodesByName(bipedNodes, FormatString("%s L Finger0*", bip));
			int numtoes = CountNodesByName(bipedNodes, FormatString("%s L Toe?", bip));
			int ntoelinks = CountNodesByName(bipedNodes, FormatString("%s L Toe0*", bip));
			BOOL prop1exists = CountNodesByName(bipedNodes, FormatString("%s Prop1", bip)) ? TRUE : FALSE;
			BOOL prop2exists = CountNodesByName(bipedNodes, FormatString("%s Prop2", bip)) ? TRUE : FALSE;
			BOOL prop3exists = CountNodesByName(bipedNodes, FormatString("%s Prop3", bip)) ? TRUE : FALSE;
			int forearmTwistLinks = CountNodesByName(bipedNodes, FormatString("%s L Fore*Twist*", bip));
			int upperarmTwistLinks = CountNodesByName(bipedNodes, FormatString("%s L Up*Twist*", bip));
			int thighTwistLinks = CountNodesByName(bipedNodes, FormatString("%s L Thigh*Twist*", bip));
			int calfTwistLinks = CountNodesByName(bipedNodes, FormatString("%s L Calf*Twist*", bip));
			int horseTwistLinks = CountNodesByName(bipedNodes, FormatString("%s L Horse*Twist*", bip));

			const float baseThigh = 9.758703f;
			const float baseFoot = 32.016972f;
			const float baseToe = 12.001712f;
			// Use the thigh as a baseline for object scaling.
			float thighScale = FindNodeByName( nodes, FormatString("%s R Thigh", bip))->GetLocalTranslation().Magnitude() / baseThigh;
			float footScale = FindNodeByName( nodes, FormatString("%s R Foot", bip))->GetLocalTranslation().Magnitude() / baseFoot;
			float toeScale = FindNodeByName( nodes, FormatString("%s R Toe0", bip))->GetLocalTranslation().Magnitude() / baseToe;

			float scale = (thighScale + footScale + toeScale) / 3.0f;

			
			Matrix44 rotX90 (+1.0f,0.0f,0.0f,0.0f, 0.0f,0.0f,+1.0f,0.0f, 0.0f,-1.0f,0.0f,0.0f, 0.0f,0.0f,0.0f,0.0f);
			Matrix44 rotXM90(+1.0f,0.0f,0.0f,0.0f, 0.0f,0.0f,-1.0f,0.0f, 0.0f,+1.0f,0.0f,0.0f, 0.0f,0.0f,0.0f,0.0f);

			Matrix44 rotY90 (0.0f,0.0f,-1.0f,0.0f, 0.0f,+1.0f,0.0f,0.0f, +1.0f,0.0f,0.0f,0.0f, 0.0f,0.0f,0.0f,0.0f);
			Matrix44 rotYM90(0.0f,0.0f,+1.0f,0.0f, 0.0f,+1.0f,0.0f,0.0f, -1.0f,0.0f,0.0f,0.0f, 0.0f,0.0f,0.0f,0.0f);

			Matrix44 rotZ90 (0.0f,+1.0f,0.0f,0.0f, -1.0f,0.0f,0.0f,0.0f, 0.0f,0.0f,+1.0f,0.0f, 0.0f,0.0f,0.0f,0.0f);
			Matrix44 rotZM90(0.0f,-1.0f,0.0f,0.0f, +1.0f,0.0f,0.0f,0.0f, 0.0f,0.0f,+1.0f,0.0f, 0.0f,0.0f,0.0f,0.0f);

			// Start writing out the biped
         int version = userver == 0 ? 45 : userver;
			out.writeLine("VERSION");
			out.writeLine(version);
			
			// Number of limbs
			out.writeLine(10);
			out.writeLine(0);
			out.writeLine(arms ? 2 : 0);

			// Neck & Spine
			out.writeLine(11);
			{
				out.writeLine(3 * nspinelinks);
				for (int i=nnecklinks-1; i>=0; --i){
					NiNodeRef link = FindNodeByName( nodes, i==0 ? FormatString("%s Neck", bip) : FormatString("%s Neck%d", bip, i) );
					out.writeLine( link->GetLocalTranslation().Magnitude() );
				}
				Matrix44 tm;
				NiNodeRef link = FindNodeByName( nodes, nspinelinks==1 ? FormatString("%s Spine", bip) : FormatString("%s Spine%d", bip, nspinelinks-1) ) ;
				tm[3][2] = link->GetLocalTranslation().Magnitude();
				out.writeLine( transRotate(tm) );
			}

			// Head Nub
			out.writeLine(12);
			{
				NiNodeRef head = FindNodeByName( nodes, FormatString("%s Head", bip) ) ;
				out.writeLine( head->GetLocalTranslation().Magnitude() * 3.1333333 ); 
				//TODO: ??? how to calculate this guy??  From camera?
			}

			// Head & neck
			out.writeLine(13);
			{
				out.writeLine(3 * nnecklinks);
 
				NiNodeRef head = FindNodeByName( nodes, FormatString("%s Head", bip) ) ;
				NiNodeRef top = head;
				for (int i=nnecklinks-2; i>=1; ++i){
					NiNodeRef link = FindNodeByName( nodes, FormatString("%s Neck%d", bip, i) );
					out.writeLine( (link->GetLocalTranslation() - top->GetLocalTranslation()).Magnitude() );
					top = link;
				}
				out.writeLine( head->GetLocalTranslation().Magnitude() );

				out.writeLine( transRotate( Matrix44(head->GetLocalRotation()) ) );
			}

			// Legs
			out.writeLine(15);
			{
				// Right Leg
				out.writeLine(nleglinks - 3);
				out.writeLine(6.402909 * thighScale); //TODO:  Unknown value
				if (nleglinks == 3)
				{
					out.writeLine( FindNodeByName( nodes, FormatString("%s R Calf", bip))->GetLocalTranslation().Magnitude() );
					out.writeLine( FindNodeByName( nodes, FormatString("%s R Foot", bip))->GetLocalTranslation().Magnitude() );
					out.writeLine( 0.096386 ); // Horse leg length
				}
				else
				{
					out.writeLine( FindNodeByName( nodes, FormatString("%s R Calf", bip))->GetLocalTranslation().Magnitude()  );
					out.writeLine( FindNodeByName( nodes, FormatString("%s R HorseLink", bip))->GetLocalTranslation().Magnitude() );
					out.writeLine( FindNodeByName( nodes, FormatString("%s R Foot", bip))->GetLocalTranslation().Magnitude() ); 
				}
				out.writeLine( 0.036145 );
				out.writeLine( 6.082767 * footScale ); // width of foot
				out.writeLine( 7.363348 * footScale ); // height of foot
				out.writeLine( 2.369076 * footScale ); // length of foot
				out.writeLine( 9.476304 * footScale ); // y-axis offset of foot object

				out.writeLine( 1 );
				out.writeLine( 1 );

				// Toe Rotation?
				Matrix44 rm( 0.000000f, 0.000000f, -1.000000f, 0.000000f,
							 0.000000f, 1.000000f, 0.000000f, 0.000000f,
							 1.000000f, 0.000000f, 0.000000f, 0.000000f,
							 5.582526f, 0.000000f, 7.184465f, 1.000000f);
				Vector3 vrtoe = FindNodeByName( nodes, FormatString("%s R Toe0", bip))->GetLocalTranslation();
				rm[2][3] = vrtoe[0];
				rm[1][3] = vrtoe[1];
				rm[0][3] = vrtoe[2];
				out.writeLine(rm);
				out.writeLine( 0.960436 * toeScale ); // Toe Nub Length

				// Left Leg
				out.writeLine(nleglinks - 3);
				out.writeLine(6.402909 * thighScale); //TODO:  Unknown value
				if (nleglinks == 3)
				{
					out.writeLine( FindNodeByName( nodes, FormatString("%s L Calf", bip))->GetLocalTranslation().Magnitude() );
					out.writeLine( FindNodeByName( nodes, FormatString("%s L Foot", bip))->GetLocalTranslation().Magnitude() );
					out.writeLine( 0.096386 ); // Horse leg length
				}
				else
				{
					out.writeLine( FindNodeByName( nodes, FormatString("%s L Calf", bip))->GetLocalTranslation().Magnitude()  );
					out.writeLine( FindNodeByName( nodes, FormatString("%s L HorseLink", bip))->GetLocalTranslation().Magnitude() );
					out.writeLine( FindNodeByName( nodes, FormatString("%s L Foot", bip))->GetLocalTranslation().Magnitude() ); 
				}
				out.writeLine( 0.036145 );
				out.writeLine( 6.082767 * footScale ); // width of foot
				out.writeLine( 7.363348 * footScale ); // height of foot
				out.writeLine( 2.369076 * footScale ); // length of foot
				out.writeLine( 9.476304 * footScale ); // y-axis offset of foot object
				out.writeLine( 1 );
				out.writeLine( 1 );

				// Toe Rotation and Position

				Matrix44 lm(-1.000000f, 0.000000f, 0.000000f, 5.582526f,
							 0.000000f, 0.000000f, 1.000000f, 7.184465f,
							-0.000000f,-1.000000f, 0.000000f, 0.000000f,
							 0.000000f, 0.000000f, 0.000000f, 1.000000f);
				Vector3 vltoe = FindNodeByName( nodes, FormatString("%s L Toe0", bip))->GetLocalTranslation();
				lm[2][3] = vltoe[0];
				lm[1][3] = vltoe[1];
				lm[0][3] = vltoe[2];
				out.writeLine(lm);
				out.writeLine(0.960436 * toeScale ); // Toe Nub Length

			}

			// Arms
			out.writeLine(16);
			{
			}

			// ????
			out.writeLine(19);
			{
				out.writeLine(0);
			}

			// ????
			out.writeLine(20);
			{
				out.writeLine(0);
			}

			// ????
			if (version >= 49)
			{
				out.writeLine(30);
				{
					out.writeLine(0);
				}
			}

			// ????
			out.writeLine(21);
			{
				out.writeLine(0);
			}

			// ????
			out.writeLine(22);
			{
				out.writeLine(0);
			}

			// ????
			out.writeLine(23);
			{
				out.writeLine(0);
			}

			// ????
			out.writeLine(25);
			{
				out.writeLine(0);
				out.writeLine(0);
				out.writeLine(0);
				out.writeLine(0);
				out.writeLine(0);
				out.writeLine(0);
				out.writeLine(0);
				out.writeLine(0);
				out.writeLine(0);
				out.writeLine(0);
				out.writeLine(0);
				out.writeLine(0.500000);
				out.writeLine(0.500000);
				out.writeLine(0.500000);
				out.writeLine(0.500000);
				out.writeLine(0.500000);
				out.writeLine(0.500000);
				out.writeLine(0.500000);
				out.writeLine(0.500000);
				out.writeLine(0.500000);
				out.writeLine(0.500000);
			}

			// ????
			out.writeLine(26);
			{
				out.writeLine(0);
				out.writeLine(0);
				out.writeLine(0);
				out.writeLine(0);
				out.writeLine(0);
				out.writeLine(0);
				out.writeLine(1);
				out.writeLine(1);
				out.writeLine(0);
			}

			// ????
			out.writeLine(18);
			{
				out.writeLine(0.000000);
			}

			// ????
			out.writeLine(17);
			{
				// Neck - Objects
				out.writeLine(501);
				{
					NiNodeRef head = FindNodeByName( nodes, FormatString("%s Head", bip) ) ;
					float value = head->GetLocalTranslation().Magnitude();
					out.writeLine( value * 1.73333 ); // y-axis
					out.writeLine( value * 1.73333 ); // x-axis

					for (int i=nnecklinks-2; i>=1; ++i){
						NiNodeRef link = FindNodeByName( nodes, FormatString("%s Neck%d", bip, i) );
						float value = (link->GetLocalTranslation()).Magnitude();
						out.writeLine( value ); // y-axis
						out.writeLine( value ); // x-axis
						head = link;
					}
				}

				// Spine
				out.writeLine(503);
				{
					NiNodeRef top = FindNodeByName( nodes, FormatString("%s Neck", bip));
					for (int i=nnecklinks-2; i>=1; ++i){
						NiNodeRef link = FindNodeByName( nodes, FormatString("%s Spine%d", bip, i) );
						float value = (link->GetLocalTranslation() - top->GetLocalTranslation()).Magnitude();
						out.writeLine( value ); // y-axis
						out.writeLine( value ); // x-axis
						top = link;
					}
					NiNodeRef link = FindNodeByName( nodes, FormatString("%s Spine", bip) );
					float value = (link->GetLocalTranslation() - top->GetLocalTranslation()).Magnitude();
					out.writeLine( value * 1.28395 ); // y-axis
					out.writeLine( value * 1.28395 ); // x-axis
				}

				out.writeLine(506);
				{
				}

				out.writeLine(505);
				{

					out.writeLine(32.014542 * scale);
					out.writeLine(32.014542 * scale);
					out.writeLine(32.014542 * scale);
					out.writeLine(32.014542 * scale);

					// lower calf
					if (nleglinks == 4)
					{
						out.writeLine( FindNodeByName( nodes, FormatString("%s L Foot", bip))->GetLocalTranslation().Magnitude() ); 
						out.writeLine( FindNodeByName( nodes, FormatString("%s L Foot", bip))->GetLocalTranslation().Magnitude() ); 
						out.writeLine(0.000000);
						out.writeLine(0.000000);
					}

					out.writeLine(0.960436 * scale);
					out.writeLine(0.960436 * scale);
					out.writeLine(40.000000);
					out.writeLine(40.000000);
					out.writeLine(40.000000);

					out.writeLine(32.014542 * scale);
					out.writeLine(32.014542 * scale);
					out.writeLine(32.014542 * scale);
					out.writeLine(32.014542 * scale);

					// lower calf
					if (nleglinks == 4)
					{
						out.writeLine( FindNodeByName( nodes, FormatString("%s R Foot", bip))->GetLocalTranslation().Magnitude() ); 
						out.writeLine( FindNodeByName( nodes, FormatString("%s R Foot", bip))->GetLocalTranslation().Magnitude() ); 
						out.writeLine(0.000000);
						out.writeLine(0.000000);
					}

					out.writeLine(0.960436 * scale);
					out.writeLine(0.960436 * scale);
					out.writeLine(40.000000);
					out.writeLine(40.000000);
					out.writeLine(40.000000);
				}

				out.writeLine(500);
				{
					out.writeLine(15.046841 * scale);
					out.writeLine(15.046841 * scale);
					out.writeLine(13.926325 * scale);
					out.writeLine(15.046841 * scale);
					out.writeLine(16.007271 * scale);
					out.writeLine(16.007271 * scale);
					out.writeLine(6.402909 * scale);
				}

				out.writeLine(507);
				{
					// Head Animation
					out.writeLine(100);
					{
						NiNodeRef link = FindNodeByName( nodes, FormatString("%s Head", bip) );
						Quaternion q = link->GetLocalTransform().Inverse().GetRotation().AsQuaternion();
						out.writeLine(q.z);
						out.writeLine(q.y);
						out.writeLine(q.x);
						out.writeLine(q.w);

						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);

						// Neck ...
						out.writeLine(3 * nnecklinks);
						for (int i=0; i<nnecklinks; ++i) {
							out.writeLine(0.000000);
							out.writeLine(0.000000);
							out.writeLine(0.000000);
						}
						if (version >= 49)
							out.writeLine(Vector4(0.000000f,0.000000f,0.000000f,0.000000f));
						out.writeLine(0.000000);
						if (version >= 49)
							out.writeLine(Vector4(0.000000f,0.000000f,0.000000f,1.000000f));
					}

					// Neck Animation
					out.writeLine(101);
					{
						out.writeLine(3 * nnecklinks);

						NiNodeRef link = FindNodeByName( nodes, FormatString("%s Neck", bip) );
						Vector3 ang = TOEULER(link->GetLocalTransform().Inverse().GetRotation());
						out.writeLine(ang[0]);
						out.writeLine(ang[1]);
						out.writeLine(ang[2]);

						for (int i=1; i<nnecklinks; ++i) {
							NiNodeRef link = FindNodeByName( nodes, FormatString("%s Neck%d", bip, i) );
							Vector3 ang = TOEULER(link->GetLocalTransform().Inverse().GetRotation());
							out.writeLine(ang[0]);
							out.writeLine(ang[1]);
							out.writeLine(ang[2]);
						}

						if (version >= 49)
							out.writeLine(Vector4(0.000000,0.000000,0.000000,0.000000));

						// Transform is pelvis position, 31.347443 for triangle, else -0.000259 
						//Matrix44 tm( 0.000001f,  0.000000f,  1.000000f, 0.613510f,
						//			 0.000000f,  1.000000f,  0.000000f, 53.883694f,
						//			-1.000000f,  0.000000f,  0.000001f,  -0.000259f,
						//			 0.000000f,  0.000000f,  0.000000f,  1.000000f);
						//tm[1][3] = 71.072556f * scale;
						Matrix44 tm = FindNodeByName( nodes, bip )->GetWorldTransform();
						out.writeLine( transRotate(tm) );
					}

					// Bip Root
					out.writeLine(108);
					{
						NiNodeRef link = FindNodeByName( nodes, bip );
						Matrix44 tm = transRotate(link->GetWorldTransform());
						//Vector4 v4(tm.GetTranslation()); v4[3] = 1.0f;
						out.writeLine(Vector4(0.000000f, 0.000000f, 0.000000f, 0.000000f));
						out.writeLine( tm[3] );

						Quaternion q = tm.GetRotation().AsQuaternion();
						out.writeLine(q.x);
						out.writeLine(q.y);
						out.writeLine(q.z);
						out.writeLine(-q.w);
					}

               // Pelvis
					out.writeLine(102);
					{
                  NiNodeRef link = FindNodeByName( nodes, FormatString("%s Pelvis", bip) );
                  Matrix44 tm = transRotate(link->GetLocalTransform());
                  Quaternion q = tm.GetRotation().AsQuaternion();
                  out.writeLine(q.x);
                  out.writeLine(q.y);
                  out.writeLine(q.z);
                  out.writeLine(-q.w);

                  // Pelvis rotations
						out.writeLine(-0.000000);
						out.writeLine(-0.000000);
						out.writeLine(0.000000);
						out.writeLine(1.000000);

						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
					}

					out.writeLine(103); // Spine
					{
						out.writeLine(3);

						Matrix33 tm = FindNodeByName( nodes, FormatString("%s Spine", bip))->GetWorldTransform().Inverse().GetRotation();
						Niflib::Vector3 v3 = TOEULER(tm);

						out.writeLine(v3[2]); // YPR
						out.writeLine(v3[1]);
						out.writeLine(v3[0]);
						if (version >= 49)
							out.writeLine(Vector4(0.000000f, 0.000000f, 0.000000f, 0.000000f));
					}

					out.writeLine(105);
					{
						out.writeLine(2.941593);
						out.writeLine(0.300000);
						if (nleglinks == 4)
						{
							out.writeLine(0.200000);
							out.writeLine(1.570796);
						}
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.049979);
						out.writeLine(0.998750);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(-0.120000);
						out.writeLine(0.000000);
						out.writeLine(0);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(Vector4(63.709213f * scale, 0.000001f, 0.000000f, 1.000000f));
						out.writeLine(Vector4(-0.048452f, -0.903619f, 0.613510f, 1.000000f));
						out.writeLine(Vector4(0.000000f, 0.000000f, 1.000000f, 0.000000f));
						out.writeLine(3);
						out.writeLine(0);
						
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.707107);
						out.writeLine(0.707107);

						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);

						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(1.000000);
						
						// Bip R Foot
						Quaternion q;
						q = (FindNodeByName( nodes, FormatString("%s R Foot", bip))->GetWorldTransform() * rotYM90).GetRotation().AsQuaternion().Inverse();
						out.writeLine(q.x);
						out.writeLine(q.y);
						out.writeLine(q.z);
						out.writeLine(q.w);
						
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(1.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(1.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(1.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(1.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(1.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(1.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(1.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(1.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(1.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(1.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0);
						out.writeLine(0);
						out.writeLine(5);
						out.writeLine(0);
						out.writeLine(0);
						out.writeLine(Vector4(0.000000f, 0.000000f, 0.000000f, 1.000000f));
						out.writeLine(Vector4(0.000000f, 0.000000f, 0.000000f, 1.000000f));
						out.writeLine(0.000000);
						out.writeLine(2.941593);
						out.writeLine(0.300000);
						if (nleglinks == 4)
						{
							out.writeLine(0.200000);
							out.writeLine(1.570796);
						}
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.049979);
						out.writeLine(0.998750);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(-0.120000);
						out.writeLine(0.000000);
						out.writeLine(0);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(Vector4(63.709213f * scale, 0.000001f, 0.000000f, 1.000000f));
						out.writeLine(Vector4(0.047933f, -0.903619f, 0.613510f, 1.000000f));
						out.writeLine(Vector4(0.000000f, 0.000000f, 1.000000f, 0.000000f));
						out.writeLine(3);
						out.writeLine(0);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.707107);
						out.writeLine(0.707107);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(1.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(1.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(1.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(1.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(1.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(1.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(1.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(1.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(1.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(1.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(1.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(1.000000);
						out.writeLine(0.000000);
						out.writeLine(0.000000);
						out.writeLine(0);
						out.writeLine(0);
						out.writeLine(5);
						out.writeLine(0);
						out.writeLine(0);
						out.writeLine(Vector4(0.000000f, 0.000000f, 0.000000f, 1.000000f));
						out.writeLine(Vector4(0.000000f, 0.000000f, 0.000000f, 1.000000f));
						out.writeLine(0.000000);
					}

					out.writeLine(106);
					{
					}

					out.writeLine(107);
					{
					}
				}
			}
			out.writeLine(27);
			{
				out.writeLine(0.500000);
				out.writeLine(0.500000);
				out.writeLine(0.500000);
				out.writeLine(0.500000);
				out.writeLine(0.500000);
				out.writeLine(0.500000);
			}

			out.writeLine(28);
			{
				out.writeLine(6);
				out.writeLine(0);
				out.writeLine(-0.004342);
				out.writeLine(-0.009040);
				out.writeLine(-0.014528);
				out.writeLine(-0.999844);
			}
		}
	}
	catch( exception & e ) 
	{
		e=e;
	}
	catch( ... ) 
	{
	}
}

static void HelpString(NifCmd::HelpType type){
   switch (type)
   {
   case NifCmd::htShort: cout << "Skel2Fig - Update Rigid Body mass properties."; break;
   case NifCmd::htLong:  
      {
         char fullName[MAX_PATH], exeName[MAX_PATH];
         GetModuleFileName(NULL, fullName, MAX_PATH);
         _splitpath(fullName, NULL, NULL, exeName, NULL);
         cout << "Usage: " << exeName << " UpdateMassProps [-opts[modifiers]]" << endl 
            << "  Normalize mesh transforms." << endl
            << endl
            << "<Switches>" << endl
            << "  i <path>          Input File" << endl
            << "  o <path>          Output File - Defaults to input file with '-out' appended" << endl
            << "  v x.x.x.x         Nif Version to write as - Defaults to input version" << endl
            << endl
            ;
      }
      break;
   }
}


static bool ExecuteCmd(NifCmdLine &cmdLine)
{
   string current_file = cmdLine.current_file;
   string outfile = cmdLine.outfile;
   unsigned outver = cmdLine.outver;
   int argc = cmdLine.argc;
   char **argv = cmdLine.argv;

   for (int i = 0; i < argc; i++)
   {
      char *arg = argv[i];
      if (arg == NULL)
         continue;
      if (arg[0] == '-' || arg[0] == '/')
      {
         switch (tolower(arg[1]))
         {
         case '\0':
            break;
         default:
            fputs( "ERROR: Unknown argument specified \"", stderr );
            fputs( arg, stderr );
            fputs( "\".\n", stderr );
            break;
         }
      }
      else if (current_file.empty())
      {
         current_file = arg;
      }
      else if (outfile.empty())
      {
         outfile = arg;
      }
   }
   if (current_file.empty()){
      NifCmd::PrintHelp();
      return false;
   }
   if (outfile.empty()){
      NifCmd::PrintHelp();
      return false;
   }

   unsigned int ver = GetNifVersion( current_file );
   if ( ver == VER_UNSUPPORTED ) cout << "unsupported...";
   else if ( ver == VER_INVALID ) cout << "invalid...";
   else 
   {
      if (!IsSupportedVersion(outver))
         outver = ver;

      // Finally alter block tree
      vector<NiObjectRef> blocks = ReadNifList( current_file );
      NiObjectRef root = blocks[0];

      FILE *hOutFile = stdout;
      if (!outfile.empty())
         hOutFile = fopen(outfile.c_str(), "wt");

      ConvertSkeletonToFig(DynamicCast<NiNode>(blocks), hOutFile, cmdLine.uoutver);

      if (!outfile.empty())
         fclose(hOutFile);

      return true;
   }
   return true;
}

REGISTER_COMMAND(Skel2Fig, HelpString, ExecuteCmd);