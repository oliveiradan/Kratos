// System includes
#include <string>
#include <iostream>
#include <cmath>

// Project includes
#include "DEM_KDEM_with_damage_parallel_bond_CL.h"
#include "custom_elements/spheric_continuum_particle.h"

namespace Kratos {

    DEMContinuumConstitutiveLaw::Pointer DEM_KDEM_with_damage_parallel_bond::Clone() const {
        DEMContinuumConstitutiveLaw::Pointer p_clone(new DEM_KDEM_with_damage_parallel_bond(*this));
        return p_clone;
    }

    void DEM_KDEM_with_damage_parallel_bond::SetConstitutiveLawInProperties(Properties::Pointer pProp, bool verbose) const {
        KRATOS_INFO("DEM") << "Assigning DEM_KDEM_with_damage_parallel_bond to Properties " << pProp->Id() << std::endl;
        pProp->SetValue(DEM_CONTINUUM_CONSTITUTIVE_LAW_POINTER, this->Clone());
        this->Check(pProp);
    }

    void DEM_KDEM_with_damage_parallel_bond::Check(Properties::Pointer pProp) const {

        DEM_KDEM_with_damage::Check(pProp);

        if (!pProp->Has(LOOSE_MATERIAL_YOUNG_MODULUS)) {
            KRATOS_WARNING("DEM")<<std::endl;
            KRATOS_WARNING("DEM")<<"WARNING: Variable LOOSE_MATERIAL_YOUNG_MODULUS was not found in the Properties when using DEM_KDEM_with_damage_parallel_bond. A default value of 0.0 was assigned."<<std::endl;
            KRATOS_WARNING("DEM")<<std::endl;
            pProp->GetValue(LOOSE_MATERIAL_YOUNG_MODULUS) = 0.0;
        }
    }

    void DEM_KDEM_with_damage_parallel_bond::CalculateElasticConstants(double& kn_el, double& kt_el, double initial_dist, double equiv_young,
                                             double equiv_poisson, double calculation_area, SphericContinuumParticle* element1, SphericContinuumParticle* element2) {

        KRATOS_TRY

        const double unbonded_equivalent_young = 0.5 * (element1->GetProperties()[LOOSE_MATERIAL_YOUNG_MODULUS] + element2->GetProperties()[LOOSE_MATERIAL_YOUNG_MODULUS]);
        const double unbonded_equivalent_shear = unbonded_equivalent_young / (2.0 * (1 + equiv_poisson));

        mUnbondedNormalElasticConstant = unbonded_equivalent_young * calculation_area / initial_dist;
        mUnbondedTangentialElasticConstant = unbonded_equivalent_shear * calculation_area / initial_dist;

        const double bonded_equiv_young = equiv_young - unbonded_equivalent_young;
        const double bonded_equiv_shear = bonded_equiv_young / (2.0 * (1 + equiv_poisson));

        kn_el = bonded_equiv_young * calculation_area / initial_dist;
        kt_el = bonded_equiv_shear * calculation_area / initial_dist;

        KRATOS_CATCH("")
    }

    void DEM_KDEM_with_damage_parallel_bond::CalculateForces(const ProcessInfo& r_process_info,
                                double OldLocalElasticContactForce[3],
                                double LocalElasticContactForce[3],
                                double LocalElasticExtraContactForce[3],
                                double LocalCoordSystem[3][3],
                                double LocalDeltDisp[3],
                                const double kn_el,
                                const double kt_el,
                                double& contact_sigma,
                                double& contact_tau,
                                double& failure_criterion_state,
                                double equiv_young,
                                double equiv_shear,
                                double indentation,
                                double calculation_area,
                                double& acumulated_damage,
                                SphericContinuumParticle* element1,
                                SphericContinuumParticle* element2,
                                int i_neighbour_count,
                                int time_steps,
                                bool& sliding,
                                int search_control,
                                DenseVector<int>& search_control_vector,
                                double &equiv_visco_damp_coeff_normal,
                                double &equiv_visco_damp_coeff_tangential,
                                double LocalRelVel[3],
                                double ViscoDampingLocalContactForce[3]) {

        KRATOS_TRY

        CalculateNormalForces(LocalElasticContactForce,
                kn_el,
                equiv_young,
                indentation,
                calculation_area,
                acumulated_damage,
                element1,
                element2,
                i_neighbour_count,
                time_steps,
                r_process_info);

        CalculateViscoDampingCoeff(equiv_visco_damp_coeff_normal,
                                   equiv_visco_damp_coeff_tangential,
                                   element1,
                                   element2,
                                   kn_el,
                                   kt_el);

        CalculateViscoDamping(LocalRelVel,
                              ViscoDampingLocalContactForce,
                              indentation,
                              equiv_visco_damp_coeff_normal,
                              equiv_visco_damp_coeff_tangential,
                              sliding,
                              element1->mIniNeighbourFailureId[i_neighbour_count]);

        CalculateTangentialForces(OldLocalElasticContactForce,
                LocalElasticContactForce,
                LocalElasticExtraContactForce,
                LocalCoordSystem,
                LocalDeltDisp,
                kt_el,
                equiv_shear,
                contact_sigma,
                contact_tau,
                indentation,
                calculation_area,
                failure_criterion_state,
                element1,
                element2,
                i_neighbour_count,
                sliding,
                search_control,
                search_control_vector,
                r_process_info);

        FindMaximumValueOfNormalAndTangentialDamageComponents();

        KRATOS_CATCH("")
    }

    void DEM_KDEM_with_damage_parallel_bond::CalculateViscoDamping(double LocalRelVel[3],
                                         double ViscoDampingLocalContactForce[3],
                                         double indentation,
                                         double equiv_visco_damp_coeff_normal,
                                         double equiv_visco_damp_coeff_tangential,
                                         bool& sliding,
                                         int failure_id) {

        KRATOS_TRY

        if ((indentation > 0) || (failure_id == 0)) {
            ViscoDampingLocalContactForce[0] = -equiv_visco_damp_coeff_tangential * LocalRelVel[0];
            ViscoDampingLocalContactForce[1] = -equiv_visco_damp_coeff_tangential * LocalRelVel[1];
            ViscoDampingLocalContactForce[2] = -equiv_visco_damp_coeff_normal * LocalRelVel[2];
        }

        mViscoDampingLocalContactForce[0] = ViscoDampingLocalContactForce[0];
        mViscoDampingLocalContactForce[1] = ViscoDampingLocalContactForce[1];
        mViscoDampingLocalContactForce[2] = ViscoDampingLocalContactForce[2];

        KRATOS_CATCH("")
    }

    void DEM_KDEM_with_damage_parallel_bond::CalculateNormalForces(double LocalElasticContactForce[3],
            const double kn_el,
            double equiv_young,
            double indentation,
            double calculation_area,
            double& acumulated_damage,
            SphericContinuumParticle* element1,
            SphericContinuumParticle* element2,
            int i_neighbour_count,
            int time_steps,
            const ProcessInfo& r_process_info) {

        KRATOS_TRY

        const double tension_limit = 0.5 * 1e6 * (element1->GetFastProperties()->GetContactSigmaMin() + element2->GetFastProperties()->GetContactSigmaMin());
        const double damage_energy_coeff = 0.5 * (element1->GetProperties()[SHEAR_ENERGY_COEF] + element2->GetProperties()[SHEAR_ENERGY_COEF]);
        const double damage_threshold_tolerance = 0.95;
        double k_unload = 0.0;
        double limit_force = 0.0;
        static bool first_time_entered = true;
        const unsigned int sphere_id = 22222222;
        const std::string filename = "normal_forces_damage.txt";

        if (element1->Id() == sphere_id) {
            static std::ifstream ifile(filename.c_str());
            if ((bool) ifile && first_time_entered) {
                std::remove("normal_forces_damage.txt");
                first_time_entered = false;
            }
        }

        if (damage_energy_coeff) {
            k_unload = kn_el / damage_energy_coeff;
        }

        int& failure_type = element1->mIniNeighbourFailureId[i_neighbour_count];
        double kn_updated = (1.0 - mDamageNormal) * kn_el;
        double current_normal_force_module = fabs(kn_updated * indentation);
        double delta_acummulated = current_normal_force_module / kn_updated;
        double returned_by_mapping_force = current_normal_force_module;

        if (indentation >= 0.0) { //COMPRESSION
            LocalElasticContactForce[2] = kn_updated * indentation;
        } else { //tension

            if (!failure_type) {

                const double initial_limit_force = tension_limit * calculation_area;
                limit_force = (1.0 - mDamageNormal) * initial_limit_force;
                LocalElasticContactForce[2] = kn_updated * indentation;

                if (current_normal_force_module > limit_force) {

                    if (!damage_energy_coeff) { // there is no damage energy left
                        failure_type = 4; // failure by traction
                    } else { // the material can sustain further damage, not failure yet
                        const double delta_at_undamaged_peak = initial_limit_force / kn_el;

                        if (kn_updated) {
                            delta_acummulated = current_normal_force_module / kn_updated;
                        } else {
                            delta_acummulated = delta_at_undamaged_peak + initial_limit_force / k_unload;
                        }

                        returned_by_mapping_force = initial_limit_force - k_unload * (delta_acummulated - delta_at_undamaged_peak);

                        if (returned_by_mapping_force < 0.0) {
                            returned_by_mapping_force = 0.0;
                        }

                        LocalElasticContactForce[2] = -returned_by_mapping_force;

                        mDamageNormal = 1.0 - (returned_by_mapping_force / delta_acummulated) / kn_el;
                        if (mDamageNormal > damage_threshold_tolerance) {
                            failure_type = 4; // failure by traction
                            KRATOS_WATCH(failure_type)
                        }
                    }
                }
            } else {
                LocalElasticContactForce[2] = 0.0;
            }
        }

        if (indentation > 0.0) {
            LocalElasticContactForce[2] += mUnbondedNormalElasticConstant * indentation;
        }

        if (element1->Id() == sphere_id) {
            static std::ofstream normal_forces_file("normal_forces_damage.txt", std::ios_base::out | std::ios_base::app);
            normal_forces_file << r_process_info[TIME] << " " << indentation << " " << LocalElasticContactForce[2] << " " << limit_force << " "
                               << delta_acummulated << " " << returned_by_mapping_force << " " << kn_updated << " " << mDamageNormal << " "
                               << failure_type << " " << current_normal_force_module << " " << mDamageTangential <<'\n';
            normal_forces_file.flush();
        }

        KRATOS_CATCH("")
    }

    void DEM_KDEM_with_damage_parallel_bond::CalculateTangentialForces(double OldLocalElasticContactForce[3],
            double LocalElasticContactForce[3],
            double LocalElasticExtraContactForce[3],
            double LocalCoordSystem[3][3],
            double LocalDeltDisp[3],
            const double kt_el,
            const double equiv_shear,
            double& contact_sigma,
            double& contact_tau,
            double indentation,
            double calculation_area,
            double& failure_criterion_state,
            SphericContinuumParticle* element1,
            SphericContinuumParticle* element2,
            int i_neighbour_count,
            bool& sliding,
            int search_control,
            DenseVector<int>& search_control_vector,
            const ProcessInfo& r_process_info) {

        KRATOS_TRY

        const double mTauZero = 0.5 * 1e6 * (GetTauZero(element1) + GetTauZero(element2));
        const double mInternalFriction = 0.5 * (GetInternalFricc(element1) + GetInternalFricc(element2));
        const double damage_energy_coeff = 0.5 * (element1->GetProperties()[SHEAR_ENERGY_COEF] + element2->GetProperties()[SHEAR_ENERGY_COEF]);
        const double damage_threshold_tolerance = 0.95;
        double k_unload = 0.0;
        double tau_strength = 0.0;
        static bool first_time_entered = true;
        int damage_process = 0;
        double maximum_frictional_shear_force = 0.0;
        const unsigned int sphere_id = 22222222;
        const std::string filename = "tangential_forces_damage.txt";

        if (element1->Id() == sphere_id) {
            static std::ifstream ifile(filename.c_str());
            if ((bool) ifile && first_time_entered) {
                std::remove("tangential_forces_damage.txt");
                first_time_entered = false;
            }
        }

        double OldBondedLocalElasticContactForce[2] = {0.0};
        OldBondedLocalElasticContactForce[0] = mBondedScalingFactor * OldLocalElasticContactForce[0];
        OldBondedLocalElasticContactForce[1] = mBondedScalingFactor * OldLocalElasticContactForce[1];

        double OldUnbondedLocalElasticContactForce[2] = {0.0};
        OldUnbondedLocalElasticContactForce[0] = mUnbondedScalingFactor * OldLocalElasticContactForce[0];
        OldUnbondedLocalElasticContactForce[1] = mUnbondedScalingFactor * OldLocalElasticContactForce[1];

        if (damage_energy_coeff) {
            k_unload = kt_el / damage_energy_coeff;
        }

        int& failure_type = element1->mIniNeighbourFailureId[i_neighbour_count];
        double kt_updated = (1.0 - mDamageTangential) * kt_el;

        double BondedLocalElasticContactForce[2] = {0.0};
        if (!failure_type) {
            BondedLocalElasticContactForce[0] = OldBondedLocalElasticContactForce[0] - kt_updated * LocalDeltDisp[0]; // 0: first tangential
            BondedLocalElasticContactForce[1] = OldBondedLocalElasticContactForce[1] - kt_updated * LocalDeltDisp[1]; // 1: second tangential
        } else {
            BondedLocalElasticContactForce[0] = OldBondedLocalElasticContactForce[0] - kt_el * LocalDeltDisp[0]; // 0: first tangential
            BondedLocalElasticContactForce[1] = OldBondedLocalElasticContactForce[1] - kt_el * LocalDeltDisp[1]; // 1: second tangential
        }
        double total_delta_displ_module = sqrt(LocalDeltDisp[0] * LocalDeltDisp[0] + LocalDeltDisp[1] * LocalDeltDisp[1]);

        double current_tangential_force_module = sqrt(BondedLocalElasticContactForce[0] * BondedLocalElasticContactForce[0]
                                                    + BondedLocalElasticContactForce[1] * BondedLocalElasticContactForce[1]);

        double delta_acummulated = current_tangential_force_module / kt_updated;

        double returned_by_mapping_force = current_tangential_force_module;

        if (!failure_type) { // This means it has not broken yet

            if (r_process_info[SHEAR_STRAIN_PARALLEL_TO_BOND_OPTION]) { //TODO: use this only for intact bonds (not broken))
                AddContributionOfShearStrainParallelToBond(OldBondedLocalElasticContactForce, LocalElasticExtraContactForce, element1->mNeighbourElasticExtraContactForces[i_neighbour_count], LocalCoordSystem, kt_el, calculation_area,  element1, element2);
            }

            contact_sigma = LocalElasticContactForce[2] / calculation_area;
            contact_tau = current_tangential_force_module / calculation_area;

            double updated_max_tau_strength = mTauZero;
            tau_strength = (1.0 - mDamageTangential) * mTauZero;

            if (contact_sigma >= 0) {
                tau_strength += (1.0 - mDamageTangential) * mInternalFriction * contact_sigma;
                updated_max_tau_strength += mInternalFriction * contact_sigma;
            }

            if (contact_tau > tau_strength) { // damage

                damage_process = 1;

                if (!damage_energy_coeff) { // there is no damage energy left
                    failure_type = 2; // failure by shear
                } else { // the material can sustain further damage, not failure yet
                    const double delta_at_undamaged_peak = updated_max_tau_strength * calculation_area / kt_el;

                    if (kt_updated) {
                        delta_acummulated = current_tangential_force_module / kt_updated;
                    } else {
                        delta_acummulated = delta_at_undamaged_peak + updated_max_tau_strength * calculation_area / k_unload;
                    }
                    if (element1->Id() == sphere_id) {
                        KRATOS_WATCH(LocalDeltDisp[0])
                        KRATOS_WATCH(OldBondedLocalElasticContactForce[0] / kt_updated)
                        KRATOS_WATCH((BondedLocalElasticContactForce[0] - OldBondedLocalElasticContactForce[0]) / kt_updated)
                        KRATOS_WATCH(BondedLocalElasticContactForce[0])
                        KRATOS_WATCH(kt_updated)
                        KRATOS_WATCH(delta_at_undamaged_peak)
                        KRATOS_WATCH(delta_acummulated)
                    }

                    returned_by_mapping_force = updated_max_tau_strength * calculation_area - k_unload * (delta_acummulated - delta_at_undamaged_peak);

                    if (element1->Id() == sphere_id) {
                        KRATOS_WATCH(delta_acummulated - delta_at_undamaged_peak)
                        KRATOS_WATCH(returned_by_mapping_force)
                    }

                    if (returned_by_mapping_force < 0.0) {
                        returned_by_mapping_force = 0.0;
                    }

                    if (current_tangential_force_module) {
                        BondedLocalElasticContactForce[0] = (returned_by_mapping_force / current_tangential_force_module) * BondedLocalElasticContactForce[0];
                        BondedLocalElasticContactForce[1] = (returned_by_mapping_force / current_tangential_force_module) * BondedLocalElasticContactForce[1];
                    }

                    current_tangential_force_module = returned_by_mapping_force; // computed only for printing purposes

                    mDamageTangential = 1.0 - (returned_by_mapping_force / delta_acummulated) / kt_el;
                    if (element1->Id() == sphere_id) {
                        KRATOS_WATCH(mDamageTangential)
                    }

                    if (mDamageTangential > damage_threshold_tolerance) {
                        failure_type = 2; // failure by shear
                    }
                }
            }
        } else {
            BondedLocalElasticContactForce[0] = OldBondedLocalElasticContactForce[0];
            BondedLocalElasticContactForce[1] = OldBondedLocalElasticContactForce[1];
        }

        // HERE STARTS THE UNBONDED PART
        double UnbondedLocalElasticContactForce[2] = {0.0};
        UnbondedLocalElasticContactForce[0] = OldUnbondedLocalElasticContactForce[0] - mUnbondedTangentialElasticConstant * LocalDeltDisp[0];
        UnbondedLocalElasticContactForce[1] = OldUnbondedLocalElasticContactForce[1] - mUnbondedTangentialElasticConstant * LocalDeltDisp[1];

        const double my_tg_of_friction_angle    = element1->GetTgOfFrictionAngle();
        const double wall_tg_of_friction_angle  = element2->GetProperties()[FRICTION];
        const double equiv_tg_of_fri_ang        = 0.5 * (my_tg_of_friction_angle + wall_tg_of_friction_angle);

        const double max_admissible_shear_force = LocalElasticContactForce[2] * equiv_tg_of_fri_ang;

        const double tangential_contact_force_0 = UnbondedLocalElasticContactForce[0] + mViscoDampingLocalContactForce[0];
        const double tangential_contact_force_1 = UnbondedLocalElasticContactForce[1] + mViscoDampingLocalContactForce[1];

        const double ActualTotalShearForce = sqrt(tangential_contact_force_0 * tangential_contact_force_0 + tangential_contact_force_1 * tangential_contact_force_1);

        if (ActualTotalShearForce > max_admissible_shear_force) {

            const double ActualElasticShearForce = sqrt(UnbondedLocalElasticContactForce[0] * UnbondedLocalElasticContactForce[0] + UnbondedLocalElasticContactForce[1] * UnbondedLocalElasticContactForce[1]);

            const double dot_product = UnbondedLocalElasticContactForce[0] * mViscoDampingLocalContactForce[0] + UnbondedLocalElasticContactForce[1] * mViscoDampingLocalContactForce[1];
            const double ViscoDampingLocalContactForceModule = sqrt(mViscoDampingLocalContactForce[0] * mViscoDampingLocalContactForce[0] +
                                                                    mViscoDampingLocalContactForce[1] * mViscoDampingLocalContactForce[1]);

            if (dot_product >= 0.0) {

                if (ActualElasticShearForce > max_admissible_shear_force) {
                    const double fraction = max_admissible_shear_force / ActualElasticShearForce;
                    UnbondedLocalElasticContactForce[0]      = UnbondedLocalElasticContactForce[0] * fraction;
                    UnbondedLocalElasticContactForce[1]      = UnbondedLocalElasticContactForce[1] * fraction;
                    mViscoDampingLocalContactForce[0] = 0.0;
                    mViscoDampingLocalContactForce[1] = 0.0;
                }
                else {
                    const double ActualViscousShearForce = max_admissible_shear_force - ActualElasticShearForce;
                    const double fraction = ActualViscousShearForce / ViscoDampingLocalContactForceModule;
                    mViscoDampingLocalContactForce[0] *= fraction;
                    mViscoDampingLocalContactForce[1] *= fraction;
                }
            }
            else {
                if (ViscoDampingLocalContactForceModule >= ActualElasticShearForce) {
                    const double fraction = (max_admissible_shear_force + ActualElasticShearForce) / ViscoDampingLocalContactForceModule;
                    mViscoDampingLocalContactForce[0] *= fraction;
                    mViscoDampingLocalContactForce[1] *= fraction;
                }
                else {
                    const double fraction = max_admissible_shear_force / ActualElasticShearForce;
                    UnbondedLocalElasticContactForce[0]      = UnbondedLocalElasticContactForce[0] * fraction;
                    UnbondedLocalElasticContactForce[1]      = UnbondedLocalElasticContactForce[1] * fraction;
                    mViscoDampingLocalContactForce[0] = 0.0;
                    mViscoDampingLocalContactForce[1] = 0.0;
                }
            }
            sliding = true;
        }

        const double local_elastic_force_modulus = sqrt(LocalElasticContactForce[0] * LocalElasticContactForce[0] +
                                                        LocalElasticContactForce[1] * LocalElasticContactForce[1]);

        if (local_elastic_force_modulus) {
            mBondedScalingFactor = (BondedLocalElasticContactForce[0] * LocalElasticContactForce[0] +
                                    BondedLocalElasticContactForce[1] * LocalElasticContactForce[1]) / (local_elastic_force_modulus * local_elastic_force_modulus);

            mUnbondedScalingFactor = (UnbondedLocalElasticContactForce[0] * LocalElasticContactForce[0] +
                                      UnbondedLocalElasticContactForce[1] * LocalElasticContactForce[1]) / (local_elastic_force_modulus * local_elastic_force_modulus);
        }

        LocalElasticContactForce[0] = BondedLocalElasticContactForce[0] + UnbondedLocalElasticContactForce[0];
        LocalElasticContactForce[1] = BondedLocalElasticContactForce[1] + UnbondedLocalElasticContactForce[1];

        if (element1->Id() == sphere_id) {
            static std::ofstream tangential_forces_file("tangential_forces_damage.txt", std::ios_base::out | std::ios_base::app);
            tangential_forces_file << r_process_info[TIME] << " " << int(failure_type) << " " << UnbondedLocalElasticContactForce[0] << " "
                                   << tau_strength << " " << delta_acummulated << " " << returned_by_mapping_force << " " << kt_updated << " "
                                   << damage_process << " " << int(sliding) << " " << contact_sigma << " " << mDamageNormal << " "
                                   << contact_tau << " " << current_tangential_force_module << " " << UnbondedLocalElasticContactForce[2] << " "
                                   << maximum_frictional_shear_force << " " << mDamageTangential << " " << LocalDeltDisp[0] << " "
                                   << total_delta_displ_module << '\n';
            tangential_forces_file.flush();
        }

        KRATOS_CATCH("")
    }

} // namespace Kratos
