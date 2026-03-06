#include <cstring>

#include "pico/stdlib.h"
#include "pico/util/queue.h"
#include "pico/multicore.h"
#include "hardware/clocks.h"
#include "hardware/interp.h"
#include "hardware/pwm.h"
#include "hardware/spi.h"
#include "hardware/irq.h"
#include "hardware/timer.h"

#include "config.hpp"
#include "voice.hpp"

namespace
{
    queue_t sound_buffer;
    volatile uint16_t rx_packet[STREAM_LENGTH];
    volatile bool packet_ready = false;
    volatile uint8_t rx_index = 0;

    uint16_t stream_packet[STREAM_LENGTH];

    float decode_float_from_words(uint16_t high, uint16_t low)
    {
        const uint32_t raw = (static_cast<uint32_t>(high) << 16) | low;
        float value = 0.0f;
        std::memcpy(&value, &raw, sizeof(value));
        return value;
    }

    uint32_t pack_pwm_stereo(float left, float right)
    {
        auto to_u16 = [](float v) -> uint16_t
        {
            if (v > 1.0f)
            {
                v = 1.0f;
            }
            if (v < -1.0f)
            {
                v = -1.0f;
            }
            const float shifted = (v + 1.0f) * 0.5f;
            return static_cast<uint16_t>(shifted * 65535.0f);
        };

        const uint16_t l = to_u16(left);
        const uint16_t r = to_u16(right);
        return (static_cast<uint32_t>(l) << 16) | r;
    }

    void apply_packet_to_voice(const uint16_t packet[STREAM_LENGTH], Voice &voice)
    {
        voice.set_vco1_phase_delta(packet[IDX_VCO1_PHASE_DELTA]);
        voice.set_vco2_phase_delta(packet[IDX_VCO2_PHASE_DELTA]);
        voice.set_vco1_wave_type(static_cast<WaveType>(packet[IDX_VCO1_WAVE] & 0x03));
        voice.set_vco2_wave_type(static_cast<WaveType>(packet[IDX_VCO2_WAVE] & 0x03));
        voice.set_vco_duty(packet[IDX_VCO1_DUTY]);
        voice.set_vco_mix(packet[IDX_VCO_MIX]);

        voice.set_vcf_type(packet[IDX_VCF_TYPE] != 0);
        voice.set_vcf_coefficient_by_index(0, decode_float_from_words(packet[IDX_VCF_B0_HI], packet[IDX_VCF_B0_LO]));
        voice.set_vcf_coefficient_by_index(1, decode_float_from_words(packet[IDX_VCF_B1_HI], packet[IDX_VCF_B1_LO]));
        voice.set_vcf_coefficient_by_index(2, decode_float_from_words(packet[IDX_VCF_B2_HI], packet[IDX_VCF_B2_LO]));
        voice.set_vcf_coefficient_by_index(3, decode_float_from_words(packet[IDX_VCF_A1_HI], packet[IDX_VCF_A1_LO]));
        voice.set_vcf_coefficient_by_index(4, decode_float_from_words(packet[IDX_VCF_A2_HI], packet[IDX_VCF_A2_LO]));

        voice.set_vca_gain_lr(packet[IDX_VCA_GAIN_L], packet[IDX_VCA_GAIN_R]);
    }

    void spi_callback()
    {
        uint16_t received = 0;
        spi_read16_blocking(SPI_PORT_SOUND, 0, &received, 1);

        if (rx_index == 0 && received > 15)
        {
            return;
        }

        rx_packet[rx_index] = received;
        rx_index = static_cast<uint8_t>((rx_index + 1) % STREAM_LENGTH);

        if (rx_index == 0)
        {
            packet_ready = true;
        }
    }

    bool timer_callback(repeating_timer_t *rt)
    {
        (void)rt;

        if (queue_is_empty(&sound_buffer))
        {
            gpio_put(PICO_DEFAULT_LED_PIN, true);
            return true;
        }

        uint32_t out_level = 0;
        queue_try_remove(&sound_buffer, &out_level);
        gpio_put(PICO_DEFAULT_LED_PIN, false);

        pwm_set_gpio_level(PIN_OUT_R, (out_level & 0xFFFFu) >> 5);
        pwm_set_gpio_level(PIN_OUT_L, (out_level >> 16) >> 5);

        return true;
    }

    void core1_entry()
    {
        static repeating_timer_t timer;
        add_repeating_timer_us(AUDIO_TIMER_PERIOD_US, &timer_callback, nullptr, &timer);
        while (true)
        {
            tight_loop_contents();
        }
    }
} // namespace

int main()
{
    set_sys_clock_khz(200000, true);
    stdio_init_all();

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    queue_init(&sound_buffer, sizeof(uint32_t), 64);

    gpio_set_function(PIN_OUT_L, GPIO_FUNC_PWM);
    gpio_set_function(PIN_OUT_R, GPIO_FUNC_PWM);
    const uint pwm_slice_l = pwm_gpio_to_slice_num(PIN_OUT_L);
    const uint pwm_slice_r = pwm_gpio_to_slice_num(PIN_OUT_R);
    pwm_set_clkdiv_int_frac(pwm_slice_l, 1, 0);
    pwm_set_clkdiv_int_frac(pwm_slice_r, 1, 0);
    pwm_set_wrap(pwm_slice_l, 2048);
    pwm_set_wrap(pwm_slice_r, 2048);
    pwm_set_mask_enabled((1u << pwm_slice_l) | (1u << pwm_slice_r));

    interp_config interp_cfg = interp_default_config();
    interp_config_set_blend(&interp_cfg, true);
    interp_set_config(interp0, 0, &interp_cfg);
    interp_cfg = interp_default_config();
    interp_config_set_signed(&interp_cfg, true);
    interp_set_config(interp0, 1, &interp_cfg);

    spi_init(SPI_PORT_SOUND, SPI_BAUDRATE_SOUND);
    spi_set_slave(SPI_PORT_SOUND, true);
    spi_set_format(SPI_PORT_SOUND, 16, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    gpio_set_function(PIN_SCK_SOUND, GPIO_FUNC_SPI);
    gpio_set_function(PIN_TX_SOUND, GPIO_FUNC_SPI);
    gpio_set_function(PIN_RX_SOUND, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS_SOUND, GPIO_FUNC_SPI);

    spi0_hw->imsc = SPI_SSPIMSC_RXIM_BITS;
    irq_set_exclusive_handler(SPI0_IRQ, spi_callback);
    irq_set_enabled(SPI0_IRQ, true);

    Voice voice1;
    Voice voice2;

    multicore_launch_core1(core1_entry);

    while (true)
    {
        if (packet_ready)
        {
            uint32_t save = save_and_disable_interrupts();
            for (uint8_t i = 0; i < STREAM_LENGTH; ++i)
            {
                stream_packet[i] = rx_packet[i];
            }
            packet_ready = false;
            restore_interrupts(save);

            const uint8_t unit_id = static_cast<uint8_t>(stream_packet[IDX_UNIT_ID]);
            if (unit_id == UNIT_1)
            {
                apply_packet_to_voice(stream_packet, voice1);
            }
            else if (unit_id == UNIT_2)
            {
                apply_packet_to_voice(stream_packet, voice2);
            }
        }

        voice1.update();
        voice2.update();

        const float mixed_l = 0.5f * (voice1.get_value_l() + voice2.get_value_l());
        const float mixed_r = 0.5f * (voice1.get_value_r() + voice2.get_value_r());

        const uint32_t out_level = pack_pwm_stereo(mixed_l, mixed_r);
        queue_add_blocking(&sound_buffer, &out_level);
    }
}
