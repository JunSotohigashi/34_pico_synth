
// This file was generated by a python script on 2024-07-26T04:40:05.446640
#ifndef TABLE_WAVE_H
#define TABLE_WAVE_H

#include "pico/stdlib.h"
#include "hardware/interp.h"
#include "fixed_point.hpp"

const int32_t wave_sine[256] = {0, 1608, 3216, 4821, 6424, 8022, 9616, 11204, 12785, 14359, 15924, 17479, 19024, 20557, 22078, 23586, 25079, 26557, 28020, 29465, 30893, 32302, 33692, 35061, 36409, 37736, 39039, 40319, 41575, 42806, 44011, 45189, 46340, 47464, 48558, 49624, 50659, 51664, 52638, 53580, 54490, 55367, 56211, 57021, 57797, 58537, 59243, 59913, 60546, 61144, 61704, 62227, 62713, 63161, 63571, 63943, 64276, 64570, 64826, 65042, 65219, 65357, 65456, 65515, 65535, 65515, 65456, 65357, 65219, 65042, 64826, 64570, 64276, 63943, 63571, 63161, 62713, 62227, 61704, 61144, 60546, 59913, 59243, 58537, 57797, 57021, 56211, 55367, 54490, 53580, 52638, 51664, 50659, 49624, 48558, 47464, 46340, 45189, 44011, 42806, 41575, 40319, 39039, 37736, 36409, 35061, 33692, 32302, 30893, 29465, 28020, 26557, 25079, 23586, 22078, 20557, 19024, 17479, 15924, 14359, 12785, 11204, 9616, 8022, 6424, 4821, 3216, 1608, 0, -1608, -3216, -4821, -6424, -8022, -9616, -11204, -12785, -14359, -15924, -17479, -19024, -20557, -22078, -23586, -25079, -26557, -28020, -29465, -30893, -32302, -33692, -35061, -36409, -37736, -39039, -40319, -41575, -42806, -44011, -45189, -46340, -47464, -48558, -49624, -50659, -51664, -52638, -53580, -54490, -55367, -56211, -57021, -57797, -58537, -59243, -59913, -60546, -61144, -61704, -62227, -62713, -63161, -63571, -63943, -64276, -64570, -64826, -65042, -65219, -65357, -65456, -65515, -65535, -65515, -65456, -65357, -65219, -65042, -64826, -64570, -64276, -63943, -63571, -63161, -62713, -62227, -61704, -61144, -60546, -59913, -59243, -58537, -57797, -57021, -56211, -55367, -54490, -53580, -52638, -51664, -50659, -49624, -48558, -47464, -46340, -45189, -44011, -42806, -41575, -40319, -39039, -37736, -36409, -35061, -33692, -32302, -30893, -29465, -28020, -26557, -25079, -23586, -22078, -20557, -19024, -17479, -15924, -14359, -12785, -11204, -9616, -8022, -6424, -4821, -3216, -1608};
const int32_t wave_saw_2[256] = {0, 2191, 4380, 6563, 8738, 10903, 13055, 15191, 17309, 19406, 21480, 23528, 25549, 27539, 29496, 31419, 33304, 35151, 36956, 38719, 40436, 42107, 43729, 45302, 46822, 48290, 49703, 51061, 52362, 53604, 54788, 55912, 56976, 57979, 58919, 59798, 60613, 61366, 62056, 62683, 63247, 63747, 64185, 64561, 64875, 65127, 65318, 65450, 65522, 65535, 65491, 65390, 65234, 65024, 64761, 64446, 64081, 63667, 63205, 62698, 62146, 61552, 60917, 60242, 59530, 58781, 57999, 57185, 56339, 55466, 54565, 53639, 52690, 51720, 50730, 49722, 48698, 47660, 46608, 45546, 44475, 43395, 42310, 41220, 40126, 39031, 37935, 36840, 35747, 34658, 33573, 32494, 31421, 30355, 29298, 28250, 27211, 26184, 25167, 24162, 23169, 22188, 21220, 20265, 19323, 18395, 17479, 16577, 15688, 14812, 13948, 13097, 12258, 11430, 10614, 9808, 9012, 8226, 7449, 6680, 5918, 5164, 4415, 3671, 2932, 2196, 1462, 731, 0, -731, -1462, -2196, -2932, -3671, -4415, -5164, -5918, -6680, -7449, -8226, -9012, -9808, -10614, -11430, -12258, -13097, -13948, -14812, -15688, -16577, -17479, -18395, -19323, -20265, -21220, -22188, -23169, -24162, -25167, -26184, -27211, -28250, -29298, -30355, -31421, -32494, -33573, -34658, -35747, -36840, -37935, -39031, -40126, -41220, -42310, -43395, -44475, -45546, -46608, -47660, -48698, -49722, -50730, -51720, -52690, -53639, -54565, -55466, -56339, -57185, -57999, -58781, -59530, -60242, -60917, -61552, -62146, -62698, -63205, -63667, -64081, -64446, -64761, -65024, -65234, -65390, -65491, -65535, -65522, -65450, -65318, -65127, -64875, -64561, -64185, -63747, -63247, -62683, -62056, -61366, -60613, -59798, -58919, -57979, -56976, -55912, -54788, -53604, -52362, -51061, -49703, -48290, -46822, -45302, -43729, -42107, -40436, -38719, -36956, -35151, -33304, -31419, -29496, -27539, -25549, -23528, -21480, -19406, -17309, -15191, -13055, -10903, -8738, -6563, -4380, -2191};
const int32_t wave_saw_4[256] = {0, 3195, 6382, 9551, 12695, 15804, 18871, 21887, 24845, 27738, 30557, 33297, 35951, 38512, 40975, 43336, 45589, 47730, 49756, 51663, 53450, 55113, 56652, 58065, 59351, 60512, 61547, 62458, 63246, 63913, 64461, 64894, 65214, 65424, 65530, 65535, 65443, 65259, 64989, 64636, 64206, 63704, 63136, 62506, 61821, 61085, 60303, 59482, 58625, 57737, 56824, 55889, 54937, 53973, 52999, 52019, 51038, 50057, 49080, 48109, 47146, 46194, 45254, 44327, 43415, 42519, 41639, 40776, 39931, 39102, 38291, 37496, 36718, 35955, 35208, 34475, 33756, 33049, 32354, 31670, 30995, 30329, 29670, 29018, 28371, 27728, 27089, 26453, 25819, 25186, 24554, 23921, 23289, 22655, 22020, 21383, 20745, 20104, 19462, 18818, 18172, 17524, 16874, 16223, 15570, 14916, 14262, 13606, 12951, 12295, 11640, 10984, 10330, 9676, 9023, 8371, 7721, 7071, 6423, 5776, 5130, 4486, 3843, 3201, 2559, 1919, 1279, 639, 0, -639, -1279, -1919, -2559, -3201, -3843, -4486, -5130, -5776, -6423, -7071, -7721, -8371, -9023, -9676, -10330, -10984, -11640, -12295, -12951, -13606, -14262, -14916, -15570, -16223, -16874, -17524, -18172, -18818, -19462, -20104, -20745, -21383, -22020, -22655, -23289, -23921, -24554, -25186, -25819, -26453, -27089, -27728, -28371, -29018, -29670, -30329, -30995, -31670, -32354, -33049, -33756, -34475, -35208, -35955, -36718, -37496, -38291, -39102, -39931, -40776, -41639, -42519, -43415, -44327, -45254, -46194, -47146, -48109, -49080, -50057, -51038, -52019, -52999, -53973, -54937, -55889, -56824, -57737, -58625, -59482, -60303, -61085, -61821, -62506, -63136, -63704, -64206, -64636, -64989, -65259, -65443, -65535, -65530, -65424, -65214, -64894, -64461, -63913, -63246, -62458, -61547, -60512, -59351, -58065, -56652, -55113, -53450, -51663, -49756, -47730, -45589, -43336, -40975, -38512, -35951, -33297, -30557, -27738, -24845, -21887, -18871, -15804, -12695, -9551, -6382, -3195};
const int32_t wave_saw_8[256] = {0, 5212, 10384, 15478, 20456, 25282, 29922, 34346, 38526, 42439, 46064, 49386, 52394, 55080, 57442, 59479, 61199, 62609, 63721, 64552, 65117, 65438, 65535, 65431, 65150, 64714, 64147, 63471, 62709, 61879, 61002, 60094, 59170, 58244, 57325, 56422, 55544, 54694, 53876, 53091, 52339, 51620, 50930, 50268, 49630, 49011, 48408, 47817, 47235, 46656, 46080, 45502, 44921, 44335, 43743, 43144, 42539, 41927, 41309, 40686, 40060, 39431, 38801, 38171, 37542, 36916, 36293, 35674, 35060, 34451, 33847, 33247, 32653, 32062, 31475, 30891, 30309, 29729, 29150, 28571, 27991, 27411, 26829, 26246, 25662, 25076, 24488, 23898, 23308, 22717, 22125, 21533, 20941, 20350, 19759, 19170, 18581, 17994, 17408, 16824, 16241, 15659, 15079, 14499, 13920, 13341, 12763, 12185, 11607, 11028, 10449, 9870, 9291, 8710, 8130, 7549, 6968, 6387, 5806, 5224, 4643, 4062, 3481, 2900, 2320, 1740, 1160, 580, 0, -580, -1160, -1740, -2320, -2900, -3481, -4062, -4643, -5224, -5806, -6387, -6968, -7549, -8130, -8710, -9291, -9870, -10449, -11028, -11607, -12185, -12763, -13341, -13920, -14499, -15079, -15659, -16241, -16824, -17408, -17994, -18581, -19170, -19759, -20350, -20941, -21533, -22125, -22717, -23308, -23898, -24488, -25076, -25662, -26246, -26829, -27411, -27991, -28571, -29150, -29729, -30309, -30891, -31475, -32062, -32653, -33247, -33847, -34451, -35060, -35674, -36293, -36916, -37542, -38171, -38801, -39431, -40060, -40686, -41309, -41927, -42539, -43144, -43743, -44335, -44921, -45502, -46080, -46656, -47235, -47817, -48408, -49011, -49630, -50268, -50930, -51620, -52339, -53091, -53876, -54694, -55544, -56422, -57325, -58244, -59170, -60094, -61002, -61879, -62709, -63471, -64147, -64714, -65150, -65431, -65535, -65438, -65117, -64552, -63721, -62609, -61199, -59479, -57442, -55080, -52394, -49386, -46064, -42439, -38526, -34346, -29922, -25282, -20456, -15478, -10384, -5212};
const int32_t wave_saw_16[256] = {0, 9234, 18239, 26801, 34727, 41859, 48082, 53323, 57557, 60805, 63124, 64606, 65367, 65535, 65245, 64625, 63796, 62856, 61888, 60949, 60077, 59289, 58587, 57960, 57392, 56862, 56349, 55837, 55310, 54764, 54194, 53604, 52998, 52385, 51771, 51163, 50566, 49982, 49413, 48855, 48307, 47763, 47220, 46676, 46126, 45570, 45009, 44443, 43874, 43304, 42736, 42171, 41611, 41055, 40504, 39956, 39410, 38864, 38318, 37770, 37220, 36668, 36113, 35558, 35001, 34446, 33891, 33339, 32788, 32240, 31693, 31147, 30602, 30056, 29510, 28962, 28414, 27864, 27314, 26763, 26213, 25663, 25114, 24566, 24019, 23473, 22928, 22383, 21838, 21292, 20746, 20199, 19652, 19104, 18556, 18008, 17461, 16914, 16367, 15821, 15276, 14731, 14186, 13641, 13095, 12550, 12004, 11458, 10912, 10365, 9819, 9273, 8727, 8181, 7636, 7090, 6545, 6000, 5455, 4909, 4364, 3819, 3273, 2727, 2182, 1636, 1091, 545, 0, -545, -1091, -1636, -2182, -2727, -3273, -3819, -4364, -4909, -5455, -6000, -6545, -7090, -7636, -8181, -8727, -9273, -9819, -10365, -10912, -11458, -12004, -12550, -13095, -13641, -14186, -14731, -15276, -15821, -16367, -16914, -17461, -18008, -18556, -19104, -19652, -20199, -20746, -21292, -21838, -22383, -22928, -23473, -24019, -24566, -25114, -25663, -26213, -26763, -27314, -27864, -28414, -28962, -29510, -30056, -30602, -31147, -31693, -32240, -32788, -33339, -33891, -34446, -35001, -35558, -36113, -36668, -37220, -37770, -38318, -38864, -39410, -39956, -40504, -41055, -41611, -42171, -42736, -43304, -43874, -44443, -45009, -45570, -46126, -46676, -47220, -47763, -48307, -48855, -49413, -49982, -50566, -51163, -51771, -52385, -52998, -53604, -54194, -54764, -55310, -55837, -56349, -56862, -57392, -57960, -58587, -59289, -60077, -60949, -61888, -62856, -63796, -64625, -65245, -65535, -65367, -64606, -63124, -60805, -57557, -53323, -48082, -41859, -34727, -26801, -18239, -9234};
const int32_t wave_saw_32[256] = {0, 17113, 32744, 45704, 55313, 61480, 64631, 65535, 65069, 64011, 62903, 62013, 61382, 60917, 60490, 60009, 59449, 58834, 58214, 57625, 57080, 56565, 56055, 55531, 54985, 54425, 53866, 53319, 52787, 52264, 51741, 51211, 50673, 50130, 49589, 49053, 48524, 47999, 47473, 46943, 46409, 45873, 45339, 44808, 44281, 43755, 43228, 42698, 42167, 41635, 41104, 40575, 40048, 39522, 38994, 38466, 37936, 37406, 36877, 36349, 35822, 35296, 34769, 34240, 33712, 33183, 32655, 32128, 31601, 31075, 30548, 30020, 29492, 28964, 28437, 27910, 27383, 26857, 26330, 25802, 25275, 24748, 24221, 23694, 23168, 22641, 22114, 21587, 21060, 20533, 20006, 19480, 18954, 18427, 17900, 17374, 16847, 16320, 15793, 15267, 14741, 14214, 13688, 13161, 12634, 12108, 11581, 11055, 10529, 10002, 9476, 8949, 8422, 7896, 7370, 6843, 6317, 5791, 5264, 4738, 4211, 3685, 3158, 2632, 2106, 1579, 1053, 526, 0, -526, -1053, -1579, -2106, -2632, -3158, -3685, -4211, -4738, -5264, -5791, -6317, -6843, -7370, -7896, -8422, -8949, -9476, -10002, -10529, -11055, -11581, -12108, -12634, -13161, -13688, -14214, -14741, -15267, -15793, -16320, -16847, -17374, -17900, -18427, -18954, -19480, -20006, -20533, -21060, -21587, -22114, -22641, -23168, -23694, -24221, -24748, -25275, -25802, -26330, -26857, -27383, -27910, -28437, -28964, -29492, -30020, -30548, -31075, -31601, -32128, -32655, -33183, -33712, -34240, -34769, -35296, -35822, -36349, -36877, -37406, -37936, -38466, -38994, -39522, -40048, -40575, -41104, -41635, -42167, -42698, -43228, -43755, -44281, -44808, -45339, -45873, -46409, -46943, -47473, -47999, -48524, -49053, -49589, -50130, -50673, -51211, -51741, -52264, -52787, -53319, -53866, -54425, -54985, -55531, -56055, -56565, -57080, -57625, -58214, -58834, -59449, -60009, -60490, -60917, -61382, -62013, -62903, -64011, -65069, -65535, -64631, -61480, -55313, -45704, -32744, -17113};
const int32_t wave_saw_64[256] = {0, 31786, 54179, 64156, 65535, 64127, 63171, 62837, 62400, 61746, 61152, 60673, 60167, 59601, 59058, 58553, 58036, 57496, 56967, 56455, 55936, 55405, 54882, 54367, 53847, 53322, 52801, 52285, 51765, 51243, 50723, 50206, 49687, 49166, 48647, 48130, 47612, 47091, 46573, 46056, 45537, 45018, 44500, 43983, 43464, 42945, 42428, 41910, 41392, 40874, 40356, 39839, 39321, 38802, 38285, 37768, 37250, 36732, 36214, 35697, 35179, 34661, 34144, 33626, 33109, 32591, 32074, 31556, 31038, 30521, 30004, 29486, 28969, 28451, 27934, 27416, 26899, 26381, 25864, 25347, 24829, 24312, 23795, 23277, 22760, 22242, 21725, 21208, 20691, 20173, 19656, 19139, 18621, 18104, 17587, 17069, 16552, 16035, 15518, 15000, 14483, 13966, 13448, 12931, 12414, 11897, 11379, 10862, 10345, 9828, 9310, 8793, 8276, 7759, 7241, 6724, 6207, 5690, 5172, 4655, 4138, 3621, 3103, 2586, 2069, 1552, 1034, 517, 0, -517, -1034, -1552, -2069, -2586, -3103, -3621, -4138, -4655, -5172, -5690, -6207, -6724, -7241, -7759, -8276, -8793, -9310, -9828, -10345, -10862, -11379, -11897, -12414, -12931, -13448, -13966, -14483, -15000, -15518, -16035, -16552, -17069, -17587, -18104, -18621, -19139, -19656, -20173, -20691, -21208, -21725, -22242, -22760, -23277, -23795, -24312, -24829, -25347, -25864, -26381, -26899, -27416, -27934, -28451, -28969, -29486, -30004, -30521, -31038, -31556, -32074, -32591, -33109, -33626, -34144, -34661, -35179, -35697, -36214, -36732, -37250, -37768, -38285, -38802, -39321, -39839, -40356, -40874, -41392, -41910, -42428, -42945, -43464, -43983, -44500, -45018, -45537, -46056, -46573, -47091, -47612, -48130, -48647, -49166, -49687, -50206, -50723, -51243, -51765, -52285, -52801, -53322, -53847, -54367, -54882, -55405, -55936, -56455, -56967, -57496, -58036, -58553, -59058, -59601, -60167, -60673, -61152, -61746, -62400, -62837, -63171, -64127, -65535, -64156, -54179, -31786};
const int32_t wave_saw_128[256] = {0, 53454, 65535, 63827, 63620, 62925, 62480, 61906, 61418, 60878, 60376, 59849, 59342, 58821, 58312, 57795, 57285, 56769, 56258, 55744, 55233, 54719, 54208, 53695, 53183, 52670, 52159, 51647, 51135, 50623, 50112, 49599, 49088, 48576, 48065, 47553, 47041, 46530, 46018, 45507, 44995, 44484, 43972, 43461, 42949, 42438, 41926, 41415, 40903, 40392, 39881, 39369, 38858, 38346, 37835, 37324, 36812, 36301, 35790, 35278, 34767, 34255, 33744, 33233, 32721, 32210, 31699, 31187, 30676, 30165, 29654, 29142, 28631, 28120, 27608, 27097, 26586, 26074, 25563, 25052, 24541, 24029, 23518, 23007, 22495, 21984, 21473, 20962, 20450, 19939, 19428, 18917, 18405, 17894, 17383, 16871, 16360, 15849, 15338, 14826, 14315, 13804, 13293, 12781, 12270, 11759, 11248, 10736, 10225, 9714, 9203, 8691, 8180, 7669, 7158, 6646, 6135, 5624, 5113, 4601, 4090, 3579, 3068, 2556, 2045, 1534, 1023, 511, 0, -511, -1023, -1534, -2045, -2556, -3068, -3579, -4090, -4601, -5113, -5624, -6135, -6646, -7158, -7669, -8180, -8691, -9203, -9714, -10225, -10736, -11248, -11759, -12270, -12781, -13293, -13804, -14315, -14826, -15338, -15849, -16360, -16871, -17383, -17894, -18405, -18917, -19428, -19939, -20450, -20962, -21473, -21984, -22495, -23007, -23518, -24029, -24541, -25052, -25563, -26074, -26586, -27097, -27608, -28120, -28631, -29142, -29654, -30165, -30676, -31187, -31699, -32210, -32721, -33233, -33744, -34255, -34767, -35278, -35790, -36301, -36812, -37324, -37835, -38346, -38858, -39369, -39881, -40392, -40903, -41415, -41926, -42438, -42949, -43461, -43972, -44484, -44995, -45507, -46018, -46530, -47041, -47553, -48065, -48576, -49088, -49599, -50112, -50623, -51135, -51647, -52159, -52670, -53183, -53695, -54208, -54719, -55233, -55744, -56258, -56769, -57285, -57795, -58312, -58821, -59342, -59849, -60376, -60878, -61418, -61906, -62480, -62925, -63620, -63827, -65535, -53454};
const int32_t wave_tri_2[256] = {0, 1608, 3216, 4821, 6424, 8022, 9616, 11204, 12785, 14359, 15924, 17479, 19024, 20557, 22078, 23586, 25079, 26557, 28020, 29465, 30893, 32302, 33692, 35061, 36409, 37736, 39039, 40319, 41575, 42806, 44011, 45189, 46340, 47464, 48558, 49624, 50659, 51664, 52638, 53580, 54490, 55367, 56211, 57021, 57797, 58537, 59243, 59913, 60546, 61144, 61704, 62227, 62713, 63161, 63571, 63943, 64276, 64570, 64826, 65042, 65219, 65357, 65456, 65515, 65535, 65515, 65456, 65357, 65219, 65042, 64826, 64570, 64276, 63943, 63571, 63161, 62713, 62227, 61704, 61144, 60546, 59913, 59243, 58537, 57797, 57021, 56211, 55367, 54490, 53580, 52638, 51664, 50659, 49624, 48558, 47464, 46340, 45189, 44011, 42806, 41575, 40319, 39039, 37736, 36409, 35061, 33692, 32302, 30893, 29465, 28020, 26557, 25079, 23586, 22078, 20557, 19024, 17479, 15924, 14359, 12785, 11204, 9616, 8022, 6424, 4821, 3216, 1608, 0, -1608, -3216, -4821, -6424, -8022, -9616, -11204, -12785, -14359, -15924, -17479, -19024, -20557, -22078, -23586, -25079, -26557, -28020, -29465, -30893, -32302, -33692, -35061, -36409, -37736, -39039, -40319, -41575, -42806, -44011, -45189, -46340, -47464, -48558, -49624, -50659, -51664, -52638, -53580, -54490, -55367, -56211, -57021, -57797, -58537, -59243, -59913, -60546, -61144, -61704, -62227, -62713, -63161, -63571, -63943, -64276, -64570, -64826, -65042, -65219, -65357, -65456, -65515, -65535, -65515, -65456, -65357, -65219, -65042, -64826, -64570, -64276, -63943, -63571, -63161, -62713, -62227, -61704, -61144, -60546, -59913, -59243, -58537, -57797, -57021, -56211, -55367, -54490, -53580, -52638, -51664, -50659, -49624, -48558, -47464, -46340, -45189, -44011, -42806, -41575, -40319, -39039, -37736, -36409, -35061, -33692, -32302, -30893, -29465, -28020, -26557, -25079, -23586, -22078, -20557, -19024, -17479, -15924, -14359, -12785, -11204, -9616, -8022, -6424, -4821, -3216, -1608};
const int32_t wave_tri_4[256] = {0, 1491, 2982, 4472, 5961, 7447, 8932, 10413, 11891, 13365, 14834, 16298, 17757, 19209, 20655, 22093, 23522, 24943, 26355, 27756, 29145, 30524, 31889, 33241, 34579, 35901, 37207, 38497, 39768, 41021, 42253, 43465, 44655, 45822, 46966, 48084, 49176, 50242, 51280, 52288, 53267, 54215, 55131, 56014, 56863, 57677, 58455, 59197, 59902, 60568, 61195, 61782, 62329, 62834, 63298, 63719, 64098, 64433, 64724, 64971, 65174, 65332, 65445, 65512, 65535, 65512, 65445, 65332, 65174, 64971, 64724, 64433, 64098, 63719, 63298, 62834, 62329, 61782, 61195, 60568, 59902, 59197, 58455, 57677, 56863, 56014, 55131, 54215, 53267, 52288, 51280, 50242, 49176, 48084, 46966, 45822, 44655, 43465, 42253, 41021, 39768, 38497, 37207, 35901, 34579, 33241, 31889, 30524, 29145, 27756, 26355, 24943, 23522, 22093, 20655, 19209, 17757, 16298, 14834, 13365, 11891, 10413, 8932, 7447, 5961, 4472, 2982, 1491, 0, -1491, -2982, -4472, -5961, -7447, -8932, -10413, -11891, -13365, -14834, -16298, -17757, -19209, -20655, -22093, -23522, -24943, -26355, -27756, -29145, -30524, -31889, -33241, -34579, -35901, -37207, -38497, -39768, -41021, -42253, -43465, -44655, -45822, -46966, -48084, -49176, -50242, -51280, -52288, -53267, -54215, -55131, -56014, -56863, -57677, -58455, -59197, -59902, -60568, -61195, -61782, -62329, -62834, -63298, -63719, -64098, -64433, -64724, -64971, -65174, -65332, -65445, -65512, -65535, -65512, -65445, -65332, -65174, -64971, -64724, -64433, -64098, -63719, -63298, -62834, -62329, -61782, -61195, -60568, -59902, -59197, -58455, -57677, -56863, -56014, -55131, -54215, -53267, -52288, -51280, -50242, -49176, -48084, -46966, -45822, -44655, -43465, -42253, -41021, -39768, -38497, -37207, -35901, -34579, -33241, -31889, -30524, -29145, -27756, -26355, -24943, -23522, -22093, -20655, -19209, -17757, -16298, -14834, -13365, -11891, -10413, -8932, -7447, -5961, -4472, -2982, -1491};
const int32_t wave_tri_8[256] = {0, 1432, 2863, 4293, 5722, 7149, 8573, 9995, 11413, 12828, 14238, 15644, 17044, 18439, 19827, 21208, 22583, 23949, 25307, 26657, 27997, 29327, 30647, 31956, 33253, 34539, 35812, 37072, 38319, 39551, 40768, 41969, 43154, 44322, 45472, 46604, 47715, 48805, 49874, 50920, 51942, 52938, 53907, 54849, 55761, 56642, 57490, 58305, 59084, 59826, 60529, 61191, 61812, 62390, 62923, 63409, 63848, 64239, 64580, 64870, 65108, 65295, 65428, 65508, 65535, 65508, 65428, 65295, 65108, 64870, 64580, 64239, 63848, 63409, 62923, 62390, 61812, 61191, 60529, 59826, 59084, 58305, 57490, 56642, 55761, 54849, 53907, 52938, 51942, 50920, 49874, 48805, 47715, 46604, 45472, 44322, 43154, 41969, 40768, 39551, 38319, 37072, 35812, 34539, 33253, 31956, 30647, 29327, 27997, 26657, 25307, 23949, 22583, 21208, 19827, 18439, 17044, 15644, 14238, 12828, 11413, 9995, 8573, 7149, 5722, 4293, 2863, 1432, 0, -1432, -2863, -4293, -5722, -7149, -8573, -9995, -11413, -12828, -14238, -15644, -17044, -18439, -19827, -21208, -22583, -23949, -25307, -26657, -27997, -29327, -30647, -31956, -33253, -34539, -35812, -37072, -38319, -39551, -40768, -41969, -43154, -44322, -45472, -46604, -47715, -48805, -49874, -50920, -51942, -52938, -53907, -54849, -55761, -56642, -57490, -58305, -59084, -59826, -60529, -61191, -61812, -62390, -62923, -63409, -63848, -64239, -64580, -64870, -65108, -65295, -65428, -65508, -65535, -65508, -65428, -65295, -65108, -64870, -64580, -64239, -63848, -63409, -62923, -62390, -61812, -61191, -60529, -59826, -59084, -58305, -57490, -56642, -55761, -54849, -53907, -52938, -51942, -50920, -49874, -48805, -47715, -46604, -45472, -44322, -43154, -41969, -40768, -39551, -38319, -37072, -35812, -34539, -33253, -31956, -30647, -29327, -27997, -26657, -25307, -23949, -22583, -21208, -19827, -18439, -17044, -15644, -14238, -12828, -11413, -9995, -8573, -7149, -5722, -4293, -2863, -1432};
const int32_t wave_tri_16[256] = {0, 1411, 2822, 4233, 5641, 7048, 8453, 9854, 11253, 12647, 14038, 15424, 16804, 18179, 19548, 20911, 22266, 23613, 24953, 26283, 27605, 28917, 30219, 31510, 32789, 34057, 35312, 36554, 37783, 38998, 40197, 41382, 42550, 43702, 44836, 45952, 47049, 48126, 49184, 50219, 51233, 52224, 53192, 54134, 55051, 55942, 56805, 57640, 58444, 59217, 59957, 60662, 61329, 61957, 62543, 63085, 63579, 64024, 64415, 64752, 65031, 65250, 65408, 65503, 65535, 65503, 65408, 65250, 65031, 64752, 64415, 64024, 63579, 63085, 62543, 61957, 61329, 60662, 59957, 59217, 58444, 57640, 56805, 55942, 55051, 54134, 53192, 52224, 51233, 50219, 49184, 48126, 47049, 45952, 44836, 43702, 42550, 41382, 40197, 38998, 37783, 36554, 35312, 34057, 32789, 31510, 30219, 28917, 27605, 26283, 24953, 23613, 22266, 20911, 19548, 18179, 16804, 15424, 14038, 12647, 11253, 9854, 8453, 7048, 5641, 4233, 2822, 1411, 0, -1411, -2822, -4233, -5641, -7048, -8453, -9854, -11253, -12647, -14038, -15424, -16804, -18179, -19548, -20911, -22266, -23613, -24953, -26283, -27605, -28917, -30219, -31510, -32789, -34057, -35312, -36554, -37783, -38998, -40197, -41382, -42550, -43702, -44836, -45952, -47049, -48126, -49184, -50219, -51233, -52224, -53192, -54134, -55051, -55942, -56805, -57640, -58444, -59217, -59957, -60662, -61329, -61957, -62543, -63085, -63579, -64024, -64415, -64752, -65031, -65250, -65408, -65503, -65535, -65503, -65408, -65250, -65031, -64752, -64415, -64024, -63579, -63085, -62543, -61957, -61329, -60662, -59957, -59217, -58444, -57640, -56805, -55942, -55051, -54134, -53192, -52224, -51233, -50219, -49184, -48126, -47049, -45952, -44836, -43702, -42550, -41382, -40197, -38998, -37783, -36554, -35312, -34057, -32789, -31510, -30219, -28917, -27605, -26283, -24953, -23613, -22266, -20911, -19548, -18179, -16804, -15424, -14038, -12647, -11253, -9854, -8453, -7048, -5641, -4233, -2822, -1411};
const int32_t wave_tri_32[256] = {0, 1404, 2808, 4211, 5613, 7012, 8410, 9804, 11195, 12583, 13966, 15345, 16719, 18087, 19449, 20804, 22152, 23493, 24825, 26149, 27464, 28769, 30064, 31349, 32622, 33883, 35132, 36368, 37590, 38799, 39993, 41171, 42333, 43479, 44608, 45718, 46810, 47883, 48936, 49967, 50977, 51964, 52929, 53868, 54783, 55671, 56533, 57366, 58169, 58942, 59683, 60391, 61064, 61701, 62300, 62858, 63375, 63846, 64270, 64640, 64953, 65203, 65386, 65498, 65535, 65498, 65386, 65203, 64953, 64640, 64270, 63846, 63375, 62858, 62300, 61701, 61064, 60391, 59683, 58942, 58169, 57366, 56533, 55671, 54783, 53868, 52929, 51964, 50977, 49967, 48936, 47883, 46810, 45718, 44608, 43479, 42333, 41171, 39993, 38799, 37590, 36368, 35132, 33883, 32622, 31349, 30064, 28769, 27464, 26149, 24825, 23493, 22152, 20804, 19449, 18087, 16719, 15345, 13966, 12583, 11195, 9804, 8410, 7012, 5613, 4211, 2808, 1404, 0, -1404, -2808, -4211, -5613, -7012, -8410, -9804, -11195, -12583, -13966, -15345, -16719, -18087, -19449, -20804, -22152, -23493, -24825, -26149, -27464, -28769, -30064, -31349, -32622, -33883, -35132, -36368, -37590, -38799, -39993, -41171, -42333, -43479, -44608, -45718, -46810, -47883, -48936, -49967, -50977, -51964, -52929, -53868, -54783, -55671, -56533, -57366, -58169, -58942, -59683, -60391, -61064, -61701, -62300, -62858, -63375, -63846, -64270, -64640, -64953, -65203, -65386, -65498, -65535, -65498, -65386, -65203, -64953, -64640, -64270, -63846, -63375, -62858, -62300, -61701, -61064, -60391, -59683, -58942, -58169, -57366, -56533, -55671, -54783, -53868, -52929, -51964, -50977, -49967, -48936, -47883, -46810, -45718, -44608, -43479, -42333, -41171, -39993, -38799, -37590, -36368, -35132, -33883, -32622, -31349, -30064, -28769, -27464, -26149, -24825, -23493, -22152, -20804, -19449, -18087, -16719, -15345, -13966, -12583, -11195, -9804, -8410, -7012, -5613, -4211, -2808, -1404};
const int32_t wave_tri_64[256] = {0, 1402, 2803, 4204, 5603, 7000, 8395, 9787, 11176, 12561, 13942, 15318, 16689, 18055, 19415, 20767, 22113, 23452, 24782, 26103, 27416, 28719, 30011, 31293, 32564, 33823, 35070, 36304, 37524, 38731, 39922, 41099, 42259, 43403, 44530, 45639, 46729, 47800, 48850, 49880, 50889, 51875, 52837, 53776, 54689, 55577, 56437, 57270, 58073, 58845, 59586, 60295, 60968, 61606, 62206, 62767, 63286, 63761, 64190, 64569, 64895, 65163, 65365, 65492, 65535, 65492, 65365, 65163, 64895, 64569, 64190, 63761, 63286, 62767, 62206, 61606, 60968, 60295, 59586, 58845, 58073, 57270, 56437, 55577, 54689, 53776, 52837, 51875, 50889, 49880, 48850, 47800, 46729, 45639, 44530, 43403, 42259, 41099, 39922, 38731, 37524, 36304, 35070, 33823, 32564, 31293, 30011, 28719, 27416, 26103, 24782, 23452, 22113, 20767, 19415, 18055, 16689, 15318, 13942, 12561, 11176, 9787, 8395, 7000, 5603, 4204, 2803, 1402, 0, -1402, -2803, -4204, -5603, -7000, -8395, -9787, -11176, -12561, -13942, -15318, -16689, -18055, -19415, -20767, -22113, -23452, -24782, -26103, -27416, -28719, -30011, -31293, -32564, -33823, -35070, -36304, -37524, -38731, -39922, -41099, -42259, -43403, -44530, -45639, -46729, -47800, -48850, -49880, -50889, -51875, -52837, -53776, -54689, -55577, -56437, -57270, -58073, -58845, -59586, -60295, -60968, -61606, -62206, -62767, -63286, -63761, -64190, -64569, -64895, -65163, -65365, -65492, -65535, -65492, -65365, -65163, -64895, -64569, -64190, -63761, -63286, -62767, -62206, -61606, -60968, -60295, -59586, -58845, -58073, -57270, -56437, -55577, -54689, -53776, -52837, -51875, -50889, -49880, -48850, -47800, -46729, -45639, -44530, -43403, -42259, -41099, -39922, -38731, -37524, -36304, -35070, -33823, -32564, -31293, -30011, -28719, -27416, -26103, -24782, -23452, -22113, -20767, -19415, -18055, -16689, -15318, -13942, -12561, -11176, -9787, -8395, -7000, -5603, -4204, -2803, -1402};
const int32_t wave_tri_128[256] = {0, 1401, 2802, 4201, 5599, 6996, 8390, 9781, 11169, 12554, 13934, 15310, 16680, 18045, 19403, 20756, 22101, 23438, 24768, 26088, 27400, 28702, 29994, 31276, 32546, 33804, 35050, 36283, 37503, 38709, 39900, 41075, 42235, 43378, 44504, 45613, 46702, 47772, 48823, 49852, 50860, 51845, 52808, 53746, 54659, 55546, 56406, 57238, 58041, 58813, 59554, 60262, 60936, 61574, 62174, 62735, 63255, 63731, 64161, 64542, 64870, 65141, 65350, 65486, 65535, 65486, 65350, 65141, 64870, 64542, 64161, 63731, 63255, 62735, 62174, 61574, 60936, 60262, 59554, 58813, 58041, 57238, 56406, 55546, 54659, 53746, 52808, 51845, 50860, 49852, 48823, 47772, 46702, 45613, 44504, 43378, 42235, 41075, 39900, 38709, 37503, 36283, 35050, 33804, 32546, 31276, 29994, 28702, 27400, 26088, 24768, 23438, 22101, 20756, 19403, 18045, 16680, 15310, 13934, 12554, 11169, 9781, 8390, 6996, 5599, 4201, 2802, 1401, 0, -1401, -2802, -4201, -5599, -6996, -8390, -9781, -11169, -12554, -13934, -15310, -16680, -18045, -19403, -20756, -22101, -23438, -24768, -26088, -27400, -28702, -29994, -31276, -32546, -33804, -35050, -36283, -37503, -38709, -39900, -41075, -42235, -43378, -44504, -45613, -46702, -47772, -48823, -49852, -50860, -51845, -52808, -53746, -54659, -55546, -56406, -57238, -58041, -58813, -59554, -60262, -60936, -61574, -62174, -62735, -63255, -63731, -64161, -64542, -64870, -65141, -65350, -65486, -65535, -65486, -65350, -65141, -64870, -64542, -64161, -63731, -63255, -62735, -62174, -61574, -60936, -60262, -59554, -58813, -58041, -57238, -56406, -55546, -54659, -53746, -52808, -51845, -50860, -49852, -48823, -47772, -46702, -45613, -44504, -43378, -42235, -41075, -39900, -38709, -37503, -36283, -35050, -33804, -32546, -31276, -29994, -28702, -27400, -26088, -24768, -23438, -22101, -20756, -19403, -18045, -16680, -15310, -13934, -12554, -11169, -9781, -8390, -6996, -5599, -4201, -2802, -1401};

inline int32_t pick_table(const int32_t wave[256], uint16_t index)
{
    interp0->base[0] = wave[index >> 8];
    interp0->base[1] = wave[(index + 1) >> 8];
    interp0->accum[1] = index & 0xFF;
    return interp0->peek[1];
}

inline int32_t mix_values(int32_t a, int32_t b, uint8_t mix = 127)
{
    interp0->base[0] = a;
    interp0->base[1] = b;
    interp0->accum[1] = mix;
    return interp0->peek[1];
}

inline Fixed_16_16 get_wave_sine(uint16_t phase16)
{
    return Fixed_16_16::from_raw_value(pick_table(wave_sine, phase16));
}

inline Fixed_16_16 get_wave_saw(uint16_t phase16, uint16_t phase16_delta)
{
    int32_t y;
    if (phase16_delta >> 15)
    {
        y = pick_table(wave_sine, phase16);
    }
    else if (phase16_delta >> 14)
    {
        y = mix_values(pick_table(wave_saw_2, phase16),
                       pick_table(wave_sine, phase16),
                       static_cast<uint8_t>(phase16_delta >> 6));
    }
    else if (phase16_delta >> 13)
    {
        y = mix_values(pick_table(wave_saw_4, phase16),
                       pick_table(wave_saw_2, phase16),
                       static_cast<uint8_t>(phase16_delta >> 5));
    }
    else if (phase16_delta >> 12)
    {
        y = mix_values(pick_table(wave_saw_8, phase16),
                       pick_table(wave_saw_4, phase16),
                       static_cast<uint8_t>(phase16_delta >> 4));
    }
    else if (phase16_delta >> 11)
    {
        y = mix_values(pick_table(wave_saw_16, phase16),
                       pick_table(wave_saw_8, phase16),
                       static_cast<uint8_t>(phase16_delta >> 3));
    }
    else if (phase16_delta >> 10)
    {
        y = mix_values(pick_table(wave_saw_32, phase16),
                       pick_table(wave_saw_16, phase16),
                       static_cast<uint8_t>(phase16_delta >> 2));
    }
    else if (phase16_delta >> 9)
    {
        y = mix_values(pick_table(wave_saw_64, phase16),
                       pick_table(wave_saw_32, phase16),
                       static_cast<uint8_t>(phase16_delta >> 1));
    }
    else if (phase16_delta >> 8)
    {
        y = mix_values(pick_table(wave_saw_128, phase16),
                       pick_table(wave_saw_64, phase16),
                       static_cast<uint8_t>(phase16_delta));
    }
    else
    {
        y = pick_table(wave_saw_128, phase16);
    }

    return Fixed_16_16::from_raw_value(y);
}

inline Fixed_16_16 get_wave_tri(uint16_t phase16, uint16_t phase16_delta)
{
    int32_t y;
    if (phase16_delta >> 15)
    {
        y = pick_table(wave_sine, phase16);
    }
    else if (phase16_delta >> 14)
    {
        y = mix_values(pick_table(wave_tri_2, phase16), pick_table(wave_sine, phase16), static_cast<uint8_t>(phase16_delta >> 6));
    }
    else if (phase16_delta >> 13)
    {
        y = mix_values(pick_table(wave_tri_4, phase16), pick_table(wave_tri_2, phase16), static_cast<uint8_t>(phase16_delta >> 5));
    }
    else if (phase16_delta >> 12)
    {
        y = mix_values(pick_table(wave_tri_8, phase16), pick_table(wave_tri_4, phase16), static_cast<uint8_t>(phase16_delta >> 4));
    }
    else if (phase16_delta >> 11)
    {
        y = mix_values(pick_table(wave_tri_16, phase16), pick_table(wave_tri_8, phase16), static_cast<uint8_t>(phase16_delta >> 3));
    }
    else if (phase16_delta >> 10)
    {
        y = mix_values(pick_table(wave_tri_32, phase16), pick_table(wave_tri_16, phase16), static_cast<uint8_t>(phase16_delta >> 2));
    }
    else if (phase16_delta >> 9)
    {
        y = mix_values(pick_table(wave_tri_64, phase16), pick_table(wave_tri_32, phase16), static_cast<uint8_t>(phase16_delta >> 1));
    }
    else if (phase16_delta >> 8)
    {
        y = mix_values(pick_table(wave_tri_128, phase16), pick_table(wave_tri_64, phase16), static_cast<uint8_t>(phase16_delta));
    }
    else
    {
        y = pick_table(wave_tri_128, phase16);
    }

    return Fixed_16_16::from_raw_value(y);
}

#endif
