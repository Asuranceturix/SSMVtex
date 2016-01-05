#include <map>
#include <iomanip>


#include "packer.h"

Packer::Packer(){

}

Packer::~Packer(){
	
}

void Packer::setInputCharts(const std::vector<Chart>& _charts){
	charts_ = _charts;
}

void Packer::getCharts(std::vector<Chart>& _charts){
	_charts = charts_;
}

float Packer::getWidth() const {
	return width_;
}

float Packer::getHeight() const {
	return height_;
}

void Packer::pack(){

    std::cerr << "Block packing started..." << std::endl;

    // Step 1: charts are rearranged with respect to their area.

    float totArea = 0.0;

    std::vector<Chart>::iterator ituw =  charts_.begin();
    std::multimap<float,Chart> ordCharts;

    for (; ituw != charts_.end(); ituw++){
        float maxArea = 0.0;

        // We calculate the area of each BBox;
        const float wside = (*ituw).getWidth();
        const float hside = (*ituw).getHeight();
        const float unwArea = wside * hside;

        // We increase the total area;
        totArea += unwArea;

        // We organize the unwraps from smallest to biggest
        ordCharts.insert(std::pair<float, Chart>(unwArea, *ituw));
    }



    // Step 2: Approximation used to calculate the image width.
    //         Image widht is fixed and height is calculated at the end

    const float nelem = (float) (charts_.size());
    int divisor = 4;
//     if (in_mode != SPLAT){
//         divisor = 4; /
//     } else {
// //        divisor = 8;
//         divisor = 2;
//     }

    const int sideElem = (int)(floor(sqrt(nelem)/divisor)); ////
    // preparation variables


    // Step 3: place the first line of charts.

    float minw = 0.0;
    std::multimap<float, Chart>::iterator itordunw;
    Vector2f vpresent (0.0,0.0);
    Vector2f vdisplace;
    std::multimap<float,Contour> contList;
    float lastw = minw;
    int counter = 0;

    for(unsigned int i = 0; i < sideElem ; i++){
        // starting from the last (biggest) unwrap of the list
        itordunw = ordCharts.end();
        itordunw--;
        float unw_h, unw_w, c_h, c_w;
        unw_h = (*itordunw).second.getHeight();
        unw_w = (*itordunw).second.getWidth();

        // if it is taller than wider, it rotates the unwrap;
        if (unw_w > unw_h){
            c_w = unw_w;
            c_h = unw_h;
        } else {
            c_w = unw_h;
            c_h = unw_w;
            (*itordunw).second.rotate();
            charts_[(*itordunw).second.getOrder()].rotate();
        }
        // the unwrap is placed in its new position
        vdisplace = vpresent;
        charts_[(*itordunw).second.getOrder()].displace(vdisplace);

        // for each unwrap placed, the new contours are updated
        Contour c;

        c.x1 = vpresent(0);
        c.x2 = vpresent(0) + c_w;
        c.y = c_h;

        // the contours are added into a list ordered by their height
        contList.insert(std::pair<float,Contour>(c.y, c));
        lastw = c.x2;
        ordCharts.erase(itordunw);
        counter++;

        // the present vector is updated
        vpresent(0) = lastw;
    }

    width_ = lastw;

    // Step 4: The rest of the charts are inserted


    while (ordCharts.size() > 0){
        // we extract the present contour: the one with lower height
        Contour cpres = (*(contList.begin())).second;
        vpresent(0) = cpres.x1;
        vpresent(1) = cpres.y;

        // we take the contour with less height and we search for an unwrap with a similar side lenght
        std::multimap<float, Chart>::iterator itun;
        // the following interator points to the unwrap with the side length most similar to the contour
        std::multimap<float, Chart>::iterator it_min_diff = ordCharts.end();
        int unwsize = ordCharts.size();

        // if the unwrap is taller than wider -> rotated = true;
        bool rotated = false;
        float cw, uw, uh, diffw, diffh, mindiff;
        mindiff = 100000.0;
        // contour width

        cw = cpres.x2 - cpres.x1;
        int s;

        // we search for the most similar side among the unwraps
        // it checks width and height and rotates it if necessary
        for(itun = ordCharts.end(), s=0; s < unwsize ; s++){
            itun--;


            uw = (*itun).second.getWidth();
            uh = (*itun).second.getHeight();
            diffw = cw - uw;
            diffh = cw - uh;

            // if the difference is minimum, we save ii
            if ((diffw > 0) && (diffw < mindiff)){
                rotated = false;
                mindiff = diffw;
                it_min_diff = itun;
            }
            if ((diffh > 0) && (diffh < mindiff)){
                rotated = true;
                mindiff = diffh;
                it_min_diff = itun;
            }
        }

        // if it does not find an unwrap which fits in the gap determined by the contour
        // the contour is deleted, and the one sharing beginning or end is updated
        if (it_min_diff == ordCharts.end()){

            std::multimap<float, Contour>::iterator itneighbor;
            std::multimap<float, Contour>::iterator itneighborLeft = contList.end();
            std::multimap<float, Contour>::iterator itneighborRight = contList.end();


            for( itneighbor = contList.begin(); itneighbor != contList.end(); itneighbor++){
                // First case: the contour shares x1 with the origin -> a right neighbor is identified
                float x2temp, x1temp;
                if (cpres.x1 == 0.0){
                    x1temp = (*itneighbor).second.x1;
                    if ( x1temp == cpres.x2){
                        itneighborRight = itneighbor;
                        break;
                    }

                    // Second case: the contour shares x1 with the maximum width -> a left neighbor is identified
                } else if (cpres.x2 == width_){
                    x2temp = (*itneighbor).second.x2;
                    if ( x2temp == cpres.x1){
                        itneighborLeft = itneighbor;
                        break;
                    }

                    // Third case: both right and left neighbors are indentified
                } else {
                    x1temp = (*itneighbor).second.x1;
                    x2temp = (*itneighbor).second.x2;
                    if ( x1temp == cpres.x2){
                        itneighborRight = itneighbor;
                    } else if ( x2temp == cpres.x1){
                        itneighborLeft = itneighbor;
                    }
                }
            }

            // There is no left neighbor -> right neighbor is updated and the previous contour is erased
            if ( itneighborLeft == contList.end() ){
                (*itneighborRight).second.x1 = cpres.x1;
                contList.erase(contList.begin());
                // There is no right neighbor -> left neighbor is updated and the previous contour is erased
            } else if ( itneighborRight == contList.end() ) {
                (*itneighborLeft).second.x2 = cpres.x2;
                contList.erase(contList.begin());
            } else {
                // here we check which neighbor has lower height -> update the neighbor contour and erase the previous one
                if ( (*itneighborLeft).second.y < (*itneighborRight).second.y ){
                    (*itneighborLeft).second.x2 = cpres.x2;
                    contList.erase(contList.begin());
                } else {
                    (*itneighborRight).second.x1 = cpres.x1;
                    contList.erase(contList.begin());
                }
            }
            // if we have found and unwraps which fits in the contuor gap
        } else {
            // we rotate it if necessary
            if (rotated){
                (*it_min_diff).second.rotate();
                charts_[(*it_min_diff).second.getOrder()].rotate();
            }
            // places the unwrap in the new position
            vdisplace = vpresent;
            charts_[(*it_min_diff).second.getOrder()].displace(vdisplace);

            Contour c1;
            c1.x1 = cpres.x1;
            c1.x2 = cpres.x1 + (*it_min_diff).second.getWidth();
            c1.y = cpres.y + (*it_min_diff).second.getHeight();

            // the contour list is updated with the new contour
            contList.insert(std::pair<float, Contour>(c1.y, c1));
            // the previous contour is updated with its new value
            (*contList.begin()).second.x1 = c1.x2;
            ordCharts.erase(it_min_diff);
            counter++;
        }

//        counter++;
          std::cerr << "\r" << (float)counter/nelem * 100 << std::setw(4) << std::setprecision(4) <<"%    " << std::flush;
    }

    // after placing all the unwraps, we can calculate the maximum height
    height_ = (*(--contList.end())).second.y;
    std::cerr << "\rdone!      " << std::endl;
}